#include "small_obj_allocator.hpp"

#include <algorithm>
#include <cassert>

namespace tcl { namespace allocators { namespace alexandrescu {

small_obj_allocator::small_obj_allocator(size_t max_obj_size)
    : last_alloc_(-1)
    , last_dealloc_(-1)
    , max_obj_size_(max_obj_size)
{
}

small_obj_allocator::small_obj_allocator(small_obj_allocator&& other)
    : pool_(std::move(other.pool_))
    , last_alloc_(other.last_alloc_)
    , last_dealloc_(other.last_alloc_)
    , max_obj_size_(other.max_obj_size_)
{
}

small_obj_allocator& small_obj_allocator::operator=(small_obj_allocator&& other)
{
    pool_ = std::move(other.pool_);
    last_alloc_ = other.last_alloc_;
    last_dealloc_ = other.last_alloc_;
    max_obj_size_ = other.max_obj_size_;

    return *this;
}

void* small_obj_allocator::allocate(size_t num_bytes)
{
    if (num_bytes > max_obj_size_)
        return new char[num_bytes];

    if (size_t(-1) == last_alloc_)
    {
        pool_.push_back(fixed_allocator(num_bytes));
        last_alloc_ = 0;
    }
    else if (pool_[last_alloc_].block_size() != num_bytes)
    {
        auto lb = std::lower_bound(pool_.begin(), pool_.end(), num_bytes);

        if (lb != pool_.end() && lb->block_size() == num_bytes)
        {
            last_alloc_ = lb - pool_.begin();
        }
        else 
        {
            pool_.push_back(fixed_allocator(num_bytes));
            std::sort(pool_.begin(), pool_.end());
            last_alloc_ = std::lower_bound(pool_.begin(), pool_.end(), num_bytes) - pool_.begin();
        }
    }

    return pool_[last_alloc_].allocate();
}

void small_obj_allocator::deallocate(void* p, size_t object_size)
{
    if (object_size > max_obj_size_)
        delete[] static_cast<char*>(p);

    assert("Ensure that we have at least one fixed allocator" && !pool_.empty());

    if (size_t(-1) == last_dealloc_ || pool_[last_dealloc_].block_size() != object_size) 
    {
        auto lb = std::lower_bound(pool_.begin(), pool_.end(), object_size);
        assert("Ensure that we found fixed_allocator" && lb->block_size() == object_size);
        last_dealloc_ = lb - pool_.begin();
    }

    pool_[last_dealloc_].deallocate(p);
}

}}}

