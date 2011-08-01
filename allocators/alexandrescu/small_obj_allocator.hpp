#pragma once

#include "fixed_allocator.hpp"

namespace tcl { namespace allocators { namespace alexandrescu {

class small_obj_allocator
{
public:
    small_obj_allocator(size_t max_obj_size = 64);

    small_obj_allocator(small_obj_allocator&& other);
    small_obj_allocator& operator=(small_obj_allocator&& other);

    void* allocate(size_t num_bytes);
    void deallocate(void* p, size_t object_size);

private:
    small_obj_allocator(const small_obj_allocator&);
    small_obj_allocator& operator=(const small_obj_allocator&);

    typedef std::vector<fixed_allocator> allocators;

    allocators pool_;
    allocators::size_type last_alloc_;
    allocators::size_type last_dealloc_;
    size_t max_obj_size_;
};

small_obj_allocator& get_allocator();

}}}
