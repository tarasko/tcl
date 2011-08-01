#pragma once

#include "fixed_pool.hpp"

#include <boost/intrusive_ptr.hpp>

#include <list>
#include <limits>

#include <stdlib.h>

namespace tcl { namespace allocators {

template<
    typename T
  , unsigned short ChunksNum = 64
  , typename FallbackAllocator = std::allocator<T>
  >
class fixed_allocator : FallbackAllocator
{
    typedef FallbackAllocator super;

public:
    typedef T value_type;

    typedef T* pointer;
    typedef T& reference;

    typedef const T* const_pointer;
    typedef const T& const_reference;

    typedef fixed_pool::size_type size_type;
    typedef fixed_pool::difference_type difference_type;

    static const unsigned short chunks_num = ChunksNum;

    template<typename T1>
    struct rebind
    {
        typedef fixed_allocator<
            T1
          , ChunksNum
          , typename FallbackAllocator::template rebind<T1>::other
          > other;
    };

    fixed_allocator();

    template<typename T1, typename FallbackAllocator1>
    fixed_allocator(const fixed_allocator<T1, ChunksNum, FallbackAllocator1>& other);

    pointer address(reference x) const;
    const_pointer address(const_reference x) const;

    pointer allocate(size_type n, void* = 0);
    void deallocate(pointer p, size_type n);

    size_type max_size() const;

    void construct(pointer p, const_reference val);
    void destroy(pointer p);

    // TODO: CRAP!
    // I want to hide this
    boost::intrusive_ptr<fixed_pool> pool() const;

private:
    boost::intrusive_ptr<fixed_pool> pool_;
};

template<typename T, unsigned short ChunksNum, typename FallbackAllocator>
fixed_allocator<T, ChunksNum, FallbackAllocator>::fixed_allocator()
{
}

template<typename T, unsigned short ChunksNum, typename FallbackAllocator>
template<typename T1, typename FallbackAllocator1>
fixed_allocator<T, ChunksNum, FallbackAllocator>::fixed_allocator(const fixed_allocator<T1, ChunksNum, FallbackAllocator1>& other)
: pool_(other.pool())
{
}

template<typename T, unsigned short ChunksNum, typename FallbackAllocator>
auto fixed_allocator<T, ChunksNum, FallbackAllocator>::address(reference x) const -> pointer
{
    return &x;
}

template<typename T, unsigned short ChunksNum, typename FallbackAllocator>
auto 
fixed_allocator<T, ChunksNum, FallbackAllocator>::address(const_reference x) const -> const_pointer
{
    return &x;
}

template<typename T, unsigned short ChunksNum, typename FallbackAllocator>
auto 
fixed_allocator<T, ChunksNum, FallbackAllocator>::allocate(size_type n, void* hint) -> pointer
{
    assert(1 == n);

    if (!pool_)
        pool_.reset(new fixed_pool(ChunksNum, sizeof(T)));
    else if (pool_->chunk_size() != n)
        return super::allocate(n, hint);

    pointer result = static_cast<pointer>(pool_->allocate());
    if (!result)
        result = super::allocate(n, hint);

    return result;
}

template<typename T, unsigned short ChunksNum, typename FallbackAllocator>
void 
fixed_allocator<T, ChunksNum, FallbackAllocator>::deallocate(pointer p, size_type n)
{
    assert(1 == n);
    assert(pool_);

    if (pool_->is_my_ptr(p))
        pool_->deallocate(p);
    else
        super::deallocate(p, n);
}

template<typename T, unsigned short ChunksNum, typename FallbackAllocator>
auto 
fixed_allocator<T, ChunksNum, FallbackAllocator>::max_size() const -> size_type
{
    // unimplemented
    return std::numeric_limits<size_type>::max();
}

template<typename T, unsigned short ChunksNum, typename FallbackAllocator>
void 
fixed_allocator<T, ChunksNum, FallbackAllocator>::construct(pointer p, const_reference val)
{
    new ((void*)p) T(val);
}

template<typename T, unsigned short ChunksNum, typename FallbackAllocator>
void 
fixed_allocator<T, ChunksNum, FallbackAllocator>::destroy(pointer p)
{
    ((T*)p)->~T();
}

template<typename T, unsigned short ChunksNum, typename FallbackAllocator>
boost::intrusive_ptr<fixed_pool> 
fixed_allocator<T, ChunksNum, FallbackAllocator>::pool() const
{
    return pool_;
}

}}
