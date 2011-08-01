#include "small_object.hpp"
#include "small_obj_allocator.hpp"

namespace tcl { namespace allocators { namespace alexandrescu {

small_obj_allocator& get_allocator()
{
    static small_obj_allocator s_allocator;
    return s_allocator;
}

void* small_object::operator new(size_t size)
{
    return get_allocator().allocate(size);
}

void small_object::operator delete(void* p, size_t size)
{
    get_allocator().deallocate(p, size);
}

small_object::~small_object()
{
}

}}}
