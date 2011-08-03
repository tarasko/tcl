#pragma once

#include <boost/atomic.hpp>
#include <boost/cstdint.hpp>

#include <memory>
#include <cassert>

namespace tcl { namespace allocators {

/// \brief Lock free pool of fixed size memory chunks.
///
/// It uses common free list algorithm to achieve O(1) complexity for \c allocate,
/// \c deallocate and \c is_my_ptr.
/// See http://en.wikipedia.org/wiki/Free_list
///
/// \tparam - Will be rebounded and used to allocate memory on construcion, and also for
/// self deallocation on destroy.
///
/// \todo - Assert in destructor that all chunks are currently free.
/// \todo - More assert in deallocate
template<typename Allocator = std::allocator<char>>
class fixed_pool : Allocator
{
    typedef Allocator allocator_type;
    typedef fixed_pool<Allocator> self_type;

    typedef typename Allocator::template rebind<char>::other char_allocator_type;
    typedef typename Allocator::template rebind<self_type>::other self_allocator_type;

public:
    // This typedefs control ability of chunk_ref to fit
    // to 4 bytes limits. boost::atomic<chunk_ref> will use spin-lock
    // version if it more then 4 bytes :(
    typedef boost::uint16_t size_type;
    typedef boost::uint16_t generation_type;

    typedef boost::int16_t difference_type;

    /// Construct pool with fixed number of fixed size chunks.
    /// Build free list upon it
    fixed_pool(size_type chunks_num, size_t chunk_size, const Allocator& allocator = Allocator());
    ~fixed_pool();

    /// Allocate one block. If there are no free block return 0.
    void* allocate();
    /// Deallocate one block. Doesn`t contain any checks in release
    /// version for efficiency. So in case of wrong p you get undefined
    /// behavior.
    void deallocate(void* p);

    /// Return chunk size
    size_t chunk_size() const;

    /// Check if chunk pointed by p belongs to this memory pool.
    bool is_my_ptr(void* p) const;

    /// Return copy of allocator
    allocator_type get_allocator() const;

private:
    friend void intrusive_ptr_add_ref(fixed_pool* p)
    {
        ++p->ref_count_;
    }

    friend void intrusive_ptr_release(fixed_pool* p)
    {
        if (p->ref_count_.fetch_sub(1) == 1)
        {
            typename fixed_pool::self_allocator_type allocator(p->get_allocator());
            p->~fixed_pool();
            allocator.deallocate(p, 1);
        }
    }

    // noncopyable, nonassignable, no move support
    fixed_pool(const fixed_pool&);
    fixed_pool(fixed_pool&& other);
    fixed_pool& operator=(const fixed_pool&);
    fixed_pool& operator=(fixed_pool&&);

    struct chunk_ref
    {
        size_type       idx_;        //!< Index of first available chunk
        generation_type generation_; //!< Resolution for ABA problem.
    };

    const size_type chunks_num_;     //!< Number of chunks in \c chunks_
    const size_t chunk_size_;        //!< Each chunk size
    const size_t total_size_;        //!< chunk_size_ * chunks_num_
    char*        chunks_;            //!< Memory block with implicit chunks

    boost::atomic<chunk_ref> head_;  //!< Index of first free chunk with generation number
    boost::atomic_int ref_count_;    //!< Reference counter for boost::intrusive_ptr
};

template<typename Allocator>
fixed_pool<Allocator>::fixed_pool(size_type chunks_num, size_t chunk_size, const Allocator& allocator)
    : Allocator(allocator)
    , chunks_num_(chunks_num)
    , chunk_size_(chunk_size > sizeof(size_type) ? chunk_size : sizeof(size_type))
    , total_size_(chunk_size_ * chunks_num_)
{
    // Allocate char array using provided self rebounded allocator
    char_allocator_type char_allocator(*this);
    chunks_ = char_allocator.allocate(total_size_);

    // Build steal index to next free block, for all blocks
    char* p = chunks_;
    size_type i = 0;

    for(; i < chunks_num_; p += chunk_size_)
        *reinterpret_cast<size_type*>(p) = ++i;

    // Make head_ to point to first free block
    chunk_ref new_head;
    new_head.idx_ = 0;
    new_head.generation_ = 0;
    head_.store(new_head, boost::memory_order_relaxed);
}

template<typename Allocator>
fixed_pool<Allocator>::~fixed_pool()
{
    char_allocator_type char_allocator(*this);
    char_allocator.deallocate(chunks_, total_size_);
}

template<typename Allocator>
void* fixed_pool<Allocator>::allocate()
{
    chunk_ref old_head = head_.load(boost::memory_order_relaxed);
    chunk_ref new_head;

    void* res;

    do {
        if (old_head.idx_ == chunks_num_)
            return 0;

        res = chunks_ + chunk_size_ * old_head.idx_;
        new_head.idx_ = *reinterpret_cast<size_type*>(res);
        new_head.generation_ = old_head.generation_ + 1;
    }
    while(!head_.compare_exchange_weak(old_head, new_head));

    return res;
}

template<typename Allocator>
void fixed_pool<Allocator>::deallocate(void* p)
{
    assert("Ensure that p doesn`t violate lower bound" && (void*)p >= chunks_.get());

    chunk_ref old_head = head_.load(boost::memory_order_relaxed);
    chunk_ref new_head;

    size_type& new_idx = *reinterpret_cast<size_type*>(p);

    do {
        new_idx = old_head.idx_;

        new_head.idx_ = (reinterpret_cast<char*>(p) - chunks_) / chunk_size_;
        new_head.generation_ = old_head.generation_ + 1;
    }
    while(!head_.compare_exchange_weak(old_head, new_head));
}

template<typename Allocator>
size_t fixed_pool<Allocator>::chunk_size() const
{
    return chunk_size_;
}

template<typename Allocator>
bool fixed_pool<Allocator>::is_my_ptr(void* p) const
{
    return p >= chunks_ && p < chunks_ + total_size_;
}

template<typename Allocator>
auto fixed_pool<Allocator>::get_allocator() const -> allocator_type
{
    return *this;
}

}}
