#pragma once

#include "fixed_pool.hpp"

#include <boost/intrusive_ptr.hpp>

#include <limits>

namespace tcl { namespace allocators {

/// \brief Fixed size, thread safe, standart compliant memory pool allocator.
///
/// All instances and copies of fixed_allocator<...> shares same memory pool.
/// Memory pool is created on first allocation attempt. After this memory pool
/// is tuned to specific object size. All attempts to allocate something that
/// cannot be satisfied by memory pool will be redirected to FallbackAllocator.
///
/// \tparam T - type of objects to allocate. Used to evaluate object size
/// \tparam ChunksNum - number of chunks in memory pool
/// \tparam FallbackAllocator - Use it if we cannot allocate from memory pool
///
/// \todo Resolve msvc problem access to other.pool_
template<
    typename T
  , unsigned short ChunksNum = 64
  , typename FallbackAllocator = std::allocator<T>
  >
class fixed_allocator : FallbackAllocator
{
    typedef FallbackAllocator super;

    typedef fixed_pool<FallbackAllocator>
        fixed_pool_type;
    typedef boost::intrusive_ptr<fixed_pool_type>
        fixed_pool_ptr;
    typedef typename FallbackAllocator::template rebind<fixed_pool_type>::other
        fixed_pool_allocator;

public:
    typedef T value_type;

    typedef T* pointer;
    typedef T& reference;

    typedef const T* const_pointer;
    typedef const T& const_reference;

    typedef typename fixed_pool_type::size_type size_type;
    typedef typename fixed_pool_type::difference_type difference_type;

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

private:
    fixed_pool_ptr pool_;
};

template<typename T, unsigned short ChunksNum, typename FallbackAllocator>
fixed_allocator<T, ChunksNum, FallbackAllocator>::fixed_allocator()
{
}

template<typename T, unsigned short ChunksNum, typename FallbackAllocator>
template<typename T1, typename FallbackAllocator1>
fixed_allocator<T, ChunksNum, FallbackAllocator>::fixed_allocator(const fixed_allocator<T1, ChunksNum, FallbackAllocator1>& other)
: pool_(other.pool_)
{
}

template<typename T, unsigned short ChunksNum, typename FallbackAllocator>
auto fixed_allocator<T, ChunksNum, FallbackAllocator>::address(reference x) const -> pointer
{
    return &x;
}

template<typename T, unsigned short ChunksNum, typename FallbackAllocator>
auto fixed_allocator<T, ChunksNum, FallbackAllocator>::address(const_reference x) const -> const_pointer
{
    return &x;
}

template<typename T, unsigned short ChunksNum, typename FallbackAllocator>
auto
fixed_allocator<T, ChunksNum, FallbackAllocator>::allocate(size_type n, void* hint) -> pointer
{
    if (!pool_)
    {
        fixed_pool_allocator allocator(static_cast<super&>(*this));
        fixed_pool_type* p = allocator.allocate(1);
        try {
            p = new (p) fixed_pool_type(ChunksNum, sizeof(T));
        }
        catch(...)
        {
            allocator.deallocate(p, 1);
            throw;
        }

        pool_.reset(p);
    }
    else if (1 != n || pool_->chunk_size() != sizeof(T))
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

}}
