#pragma once

#include <boost/atomic.hpp>

namespace tcl { namespace allocators { namespace alexandrescu {

struct chunk
{
public:
    chunk(size_t block_size, unsigned char blocks);

    // move support
    chunk(chunk&& other);
    chunk& operator=(chunk&& other);

    ~chunk();

    void* allocate(size_t block_size);
    void deallocate(void* p, size_t block_size);

    bool blocks_available() const
    {
        return blocks_available_ != 0;
    }

    bool is_my_ptr(void* ptr, size_t total_size) const
    {
        unsigned char* uc_ptr = static_cast<unsigned char*>(ptr);
        return uc_ptr >= data_ && uc_ptr < data_ + total_size;
    }

private:
    chunk(const chunk&);
    chunk& operator=(const chunk&);

    unsigned char* data_;
    unsigned char first_available_block_;
    unsigned char blocks_available_;
};

}}}
