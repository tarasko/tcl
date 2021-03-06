#pragma once

#include "fixed_pool.hpp"
#include "construct_destroy.hpp"

#include <boost/intrusive_ptr.hpp>
#include <boost/thread/once.hpp>

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

    // Rebind fallback allocator to some known type, for example for char
    typedef typename FallbackAllocator::template rebind<char>::other
        char_allocator;

    // Pass it to fixed_pool template parameter. Now we are ensured we have
    // absolutely same fixed_pool type among our rebinding. Copy constructor
    // from rebinded allocator can just copy construct fixed_pool_ptr.
    typedef fixed_pool<char_allocator>
        fixed_pool_type;

    // Smart pointer to fixed_pool
    typedef boost::intrusive_ptr<fixed_pool_type>
        fixed_pool_ptr;

    // Allocator to allocate fixed_pool itself
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
    fixed_pool_ptr pool() const;

    void construct(pointer p, const_reference val);
    void destroy(pointer p);

private:
    fixed_pool_ptr   pool_;
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
        // TODO: Here is race, if two threads call us simultaniously
        fixed_pool_allocator allocator(*(super*)this);
        pool_.reset(::tcl::allocators::construct(allocator, ChunksNum, sizeof(T)));
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
    assert("Ensure that allocate was already called" && pool_);

    if (1 == n && pool_->is_my_ptr(p))
        pool_->deallocate(p);
    else
        super::deallocate(p, n);
}

template<typename T, unsigned short ChunksNum, typename FallbackAllocator>
auto
fixed_allocator<T, ChunksNum, FallbackAllocator>::max_size() const -> size_type
{
    // unimplemented
    return std::numeric_limits<size_type>::max BOOST_PREVENT_MACRO_SUBSTITUTION();
}

template<typename T, unsigned short ChunksNum, typename FallbackAllocator>
auto
fixed_allocator<T, ChunksNum, FallbackAllocator>::pool() const -> fixed_pool_ptr
{
    return pool_;
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
