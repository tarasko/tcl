#pragma once

#include <memory>

namespace tcl { namespace allocators { namespace alexandrescu {

class small_object
{
public:
    virtual ~small_object();

    static void* operator new(size_t size);
    static void operator delete(void* p, size_t size);
};

}}}
