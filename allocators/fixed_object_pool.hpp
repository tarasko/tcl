#pragma once

#include <boost/atomic.hpp>
#include <boost/cstdint.hpp>

#include <cassert>
#include <memory>
#include <stdexcept>

namespace tcl { namespace allocators {

/// \brief This exception thrown when no more free object left in fixed_object_pool
struct no_more_objects : std::runtime_error
{
    no_more_objects() : std::runtime_error("no more objects in fixed_object_pool") {}
};

/// \brief Lock free pool of objects.
///
/// \tparam T - Object type, must be default constructible.
/// \tparam Allocator - Used to allocate memory block for objects.
///
/// \todo - Assert in destructor that all objects are currently free.
/// \todo - More assert in deallocate
/// \todo - We must distinguish scoped_allocator_adapter and in that
/// case forward allocator to T constructor
template<typename T, typename Allocator = std::allocator<char>>
class fixed_object_pool : Allocator
{
public:
    // This typedefs control ability of chunk_ref to fit
    // to 4 bytes limits. boost::atomic<chunk_ref> will use spin-lock
    // version if it more then 4 bytes :(
    typedef boost::uint16_t size_type;
    typedef boost::uint16_t generation_type;

    typedef T value_type;
    typedef T* pointer;
    typedef T& reference;

    fixed_object_pool(size_type chunks_num, const Allocator& allocator = Allocator());
    ~fixed_object_pool();

    pointer allocate();
    void deallocate(pointer p);

private:
    // noncopyable, nonassignable
    // no move support cause moving from another
    // is very difficult to make thread-safe
    fixed_object_pool(const fixed_object_pool&);
    fixed_object_pool(fixed_object_pool&& other);
    fixed_object_pool& operator=(const fixed_object_pool&);
    fixed_object_pool& operator=(fixed_object_pool&&);

    // TODO: Will proper allignment increase performance?
    // On Sparc I guess we will get exception?
    struct chunk
    {
        T obj_;
        size_type next_free_;
    };

    struct chunk_ref
    {
        size_type       idx_;        //!< Index of first available chunk
        generation_type generation_; //!< Help to resolve ABA problem.
    };

    typedef typename Allocator::template rebind<chunk>::other chunk_allocator_type;

    chunk*    chunks_;
    size_type chunks_num_;

    boost::atomic<chunk_ref> head_;  //!< Index of first free chunk with generation number
};

template<typename T, typename Allocator>
fixed_object_pool<T, Allocator>::fixed_object_pool(size_type chunks_num, const Allocator& allocator)
    : Allocator(allocator)
    , chunks_num_(chunks_num)
{
    chunk_allocator_type chunk_allocator(*this);
    chunks_ = chunk_allocator.allocate(chunks_num_);

    size_type i = 0;
    try
    {
        for(; i<chunks_num_; ++i)
        {
            new (chunks_ + i) T();
            chunks_[i].next_free_ = i + 1;
        }
    }
    catch(...)
    {
        for(size_type k=0; k<i; ++k)
            (chunks_[k].obj_).~T();
        throw;
    }

    chunk_ref new_head;
    new_head.idx_ = 0;
    new_head.generation_ = 0;
    head_.store(new_head, boost::memory_order_relaxed);
}

template<typename T, typename Allocator>
fixed_object_pool<T, Allocator>::~fixed_object_pool()
{
    for(size_type i = 0; i < chunks_num_; ++i)
        chunks_[i].obj_.~T();

    chunk_allocator_type chunk_allocator(*this);
    chunk_allocator.deallocate(chunks_, chunks_num_);
}

template<typename T, typename Allocator>
auto fixed_object_pool<T, Allocator>::allocate() -> pointer
{
    chunk_ref old_head = head_.load(boost::memory_order_relaxed);
    chunk_ref new_head;

    pointer res;

    do {
        if (old_head.idx_ == chunks_num_)
            throw no_more_objects();

        chunk& free_chunk = chunks_[old_head.idx_];

        res = reinterpret_cast<pointer>(&free_chunk);
        new_head.idx_ = free_chunk.next_free_;
        new_head.generation_ = old_head.generation_ + 1;
    }
    while(!head_.compare_exchange_weak(old_head, new_head));

    return res;
}

template<typename T, typename Allocator>
void fixed_object_pool<T, Allocator>::deallocate(pointer p)
{
    assert("Ensure that p doesn`t violate lower bound" && (void*)p >= chunks_.get());

    chunk_ref old_head = head_.load(boost::memory_order_relaxed);
    chunk_ref new_head;

    chunk& chunk_to_free = *reinterpret_cast<chunk*>(p);

    do {
        chunk_to_free.next_free_ = old_head.idx_;

        new_head.idx_ = &chunk_to_free - chunks_;
        new_head.generation_ = old_head.generation_ + 1;
    }
    while(!head_.compare_exchange_weak(old_head, new_head));
}

}}
