#pragma once

#include <boost/atomic.hpp>
#include <boost/cstdint.hpp>
#include <boost/mpl/if.hpp>

#include <cassert>
#include <memory>

namespace tcl { namespace allocators {

/// \brief Lock free pool of objects.
///
/// \tparam T - Object type
/// \tparam ConstructObject - if true then call default constructor for all
/// object after pool has been created. In this case T must be default
/// constructible.
///
/// @note Both \c allocate and \c deallocate have O(1) complexity.
template<typename T, bool ConstructObjects = false>
class fixed_object_pool
{
public:
    // this typedefs control ability of chunk_ref to fit
    // to 8 bytes limits.
    typedef boost::uint16_t size_type;
    typedef boost::uint16_t generation_type;

    typedef typename boost::mpl::if_c<
        ConstructObjects
      , T*
      , void*
      >::type pointer_type;

    fixed_object_pool(size_type chunks_num);
    ~fixed_object_pool();

    /// Return uninialized storage for object T
    /// Use placement constructor to create object
    pointer_type allocate();
    void deallocate(pointer_type p);

private:
    // noncopyable, nonassignable
    // no move support cause moving from another
    // is very difficult to make thread-safe
    fixed_object_pool(const fixed_object_pool&);
    fixed_object_pool(fixed_object_pool&& other);
    fixed_object_pool& operator=(const fixed_object_pool&);
    fixed_object_pool& operator=(fixed_object_pool&&);

    struct chunk_ref
    {
        size_type       idx_;        //!< Index of first available chunk
        generation_type generation_; //!< Help to resolve ABA problem.
    };

    // chunk size must be min(sizeof(T), sizeof(size_type))
    static const int chunk_size = sizeof(T) < sizeof(size_type) ? sizeof(size_type) : sizeof(T);

    std::unique_ptr<unsigned char[]> chunks_;
    size_type chunks_num_;

    boost::atomic<chunk_ref> head_; //!< Index of first free chunk with generation number
};

template<typename T, bool ConstructObjects>
fixed_object_pool<T, ConstructObjects>::fixed_object_pool(size_type chunks_num)
    : chunks_(new unsigned char[chunk_size * chunks_num])
    , chunks_num_(chunks_num)
{
    unsigned char* p = chunks_.get();
    size_type i = 0;

    for(; i<chunks_num; p += chunk_size)
        *reinterpret_cast<size_type*>(p) = ++i;

    chunk_ref new_head;
    new_head.idx_ = 0;
    new_head.generation_ = 0;
    head_.store(new_head, boost::memory_order_relaxed);

    // Following block will be thown out by optimizer
    if (ConstructObjects) 
    {
        unsigned char* p = chunks_.get();
        size_type i = 0;
        try 
        {
            for (; i<chunks_num; ++i, p += chunk_size)
                new (p) T();
        }
        catch(...)
        {
            // in case of exceptions call destructors for all already created objects
            p = chunks_.get();
            for (size_type k = 0; k < i; p += chunk_size)
                reinterpret_cast<pointer_type>(p)->T::~T();
        }
    }
}

template<typename T, bool ConstructObjects>
fixed_object_pool<T, ConstructObjects>::~fixed_object_pool()
{
    if (ConstructObjects) 
    {
        unsigned char* p = chunks_.get();
        size_type i = 0;

        for (; i < chunks_num_; p += chunk_size)
            reinterpret_cast<pointer_type>(p)->T::~T();
    }
}

template<typename T, bool ConstructObjects>
auto fixed_object_pool<T, ConstructObjects>::allocate() -> pointer_type
{
    chunk_ref old_head = head_.load(boost::memory_order_relaxed);
    chunk_ref new_head;

    pointer_type res;

    do {
        if (old_head.idx_ == chunks_num_)
            return 0;

        res = reinterpret_cast<pointer_type>(chunks_.get() + chunk_size * old_head.idx_);
        new_head.idx_ = *reinterpret_cast<size_type*>(res);
        new_head.generation_ = old_head.generation_ + 1;
    }
    while(!head_.compare_exchange_weak(old_head, new_head));

    return res;
}

template<typename T, bool ConstructObjects>
void fixed_object_pool<T, ConstructObjects>::deallocate(pointer_type p)
{
    assert("Ensure that p doesn`t violate lower bound" && (void*)p >= chunks_.get());

    chunk_ref old_head = head_.load(boost::memory_order_relaxed);
    chunk_ref new_head;

    size_type& new_idx = *reinterpret_cast<size_type*>(p);

    do {
        new_idx = old_head.idx_;

        new_head.idx_ = (reinterpret_cast<unsigned char*>(p) - chunks_.get()) / chunk_size;
        new_head.generation_ = old_head.generation_ + 1;
    }
    while(!head_.compare_exchange_weak(old_head, new_head));
}

}}
