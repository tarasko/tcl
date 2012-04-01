#pragma once

#include "chunk.hpp"

#include <vector>

namespace tcl { namespace allocators { namespace alexandrescu {

class fixed_allocator
{
    fixed_allocator(const fixed_allocator&);
    fixed_allocator& operator=(const fixed_allocator&);

public:
    static const unsigned char DEFAULT_NUM_BLOCKS = 255;

    explicit fixed_allocator(size_t block_size, unsigned char num_blocks = DEFAULT_NUM_BLOCKS);

    // move support
    fixed_allocator(fixed_allocator&& other);
    fixed_allocator& operator=(fixed_allocator&&);

    void* allocate();
    void deallocate(void* p);

    size_t block_size() const
    {
        return block_size_;
    }

private:
    typedef std::vector<chunk> chunks;

    size_t block_size_;
    unsigned char num_blocks_;
    size_t total_chunk_size_;
    chunks chunks_;
    
    chunks::size_type alloc_last_;
    chunks::size_type dealloc_last_;
};

inline bool operator<(const fixed_allocator& f, const fixed_allocator& s)
{
    return f.block_size() < s.block_size();
}

inline bool operator<(size_t f, const fixed_allocator& s)
{
    return f < s.block_size();
}

inline bool operator<(const fixed_allocator& f, size_t s)
{
    return f.block_size() < s;
}

}}}
