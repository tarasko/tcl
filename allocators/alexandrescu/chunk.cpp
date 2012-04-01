#include "chunk.hpp"

#include <cassert>

namespace tcl { namespace allocators { namespace alexandrescu {

chunk::chunk(size_t block_size, unsigned char blocks)
    : data_(new unsigned char[block_size * blocks])
    , first_available_block_(0)
    , blocks_available_(blocks)
{
    // Initialize stealth index
    unsigned char* p = data_;
    unsigned char i = 0;
    for (; i < blocks; p += block_size)
        *p = ++i;
}

chunk::chunk(chunk&& other)
    : data_(other.data_)
    , first_available_block_(other.first_available_block_)
    , blocks_available_(other.blocks_available_)
{
    other.data_ = 0;
}

chunk& chunk::operator=(chunk&& other)
{
    delete[] data_;
    
    data_ = other.data_;
    other.data_ = 0;

    first_available_block_= other.first_available_block_;
    blocks_available_ = other.blocks_available_;

    return *this;
}

chunk::~chunk()
{
    delete[] data_;
}

void* chunk::allocate(size_t block_size)
{
    assert("Check that we have available blocks" && blocks_available_);

    unsigned char* result = data_ + block_size * first_available_block_;

    first_available_block_ = *result;
    --blocks_available_;

    return result;
}

void chunk::deallocate(void* p, size_t block_size)
{
    assert("Parameter p must be inside of global allocated block" && p >= data_);
    unsigned char* to_release = static_cast<unsigned char*>(p);

    assert("p must be alligned according to block_size" && (to_release - data_) % block_size == 0);

    // restore stealth index
    *to_release = first_available_block_;

    first_available_block_ = static_cast<unsigned char>((to_release - data_) / block_size);

    assert("This effectively check high bound of global array" &&
        first_available_block_ == (to_release - data_) / block_size);

    ++blocks_available_;
}

}}}

