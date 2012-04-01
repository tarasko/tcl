#include "fixed_allocator.hpp"

#include <algorithm>
#include <functional>
#include <cassert>

namespace tcl { namespace allocators { namespace alexandrescu {

fixed_allocator::fixed_allocator(size_t block_size, unsigned char num_blocks)
    : block_size_(block_size)
    , num_blocks_(num_blocks)
    , total_chunk_size_(block_size_ * num_blocks_)
    , alloc_last_(0)
    , dealloc_last_(0)
{
    chunks_.push_back(chunk(block_size_, num_blocks_));
}

fixed_allocator::fixed_allocator(fixed_allocator&& other)
    : block_size_(other.block_size_)
    , num_blocks_(other.num_blocks_)
    , total_chunk_size_(other.total_chunk_size_)
    , chunks_(std::move(other.chunks_))
    , alloc_last_(0)
    , dealloc_last_(0)
{
}

fixed_allocator& fixed_allocator::operator=(fixed_allocator&& other)
{
    block_size_ = other.block_size_;
    num_blocks_ = other.num_blocks_;
    total_chunk_size_ = other.total_chunk_size_;
    chunks_ = std::move(other.chunks_);
    alloc_last_ = 0;
    dealloc_last_ = 0;

    return *this;
}

void* fixed_allocator::allocate()
{
    if (!chunks_[alloc_last_].blocks_available())
    {
        chunks::iterator i = 
            std::find_if(chunks_.begin(), chunks_.end(), std::mem_fn(&chunk::blocks_available));

        if (i == chunks_.end())
        {
            chunks_.push_back(chunk(block_size_, num_blocks_));
            alloc_last_ = chunks_.size() - 1;
        }
        else 
        {
            alloc_last_ = i - chunks_.begin();
        }
    }

    return chunks_[alloc_last_].allocate(block_size_);
}

void fixed_allocator::deallocate(void* p)
{
    if (chunks_[dealloc_last_].is_my_ptr(p, total_chunk_size_));
    // try previous chunk
    else if (dealloc_last_ != 0 && chunks_[dealloc_last_ - 1].is_my_ptr(p, total_chunk_size_))
    {
        --dealloc_last_;
    }
    // try next chunk
    else if (dealloc_last_ != chunks_.size() - 1 && chunks_[dealloc_last_ + 1].is_my_ptr(p, total_chunk_size_))
    {
        ++dealloc_last_;
    }
    // try linear search
    else 
    {
        chunks::const_iterator i = std::find_if(
            chunks_.begin()
          , chunks_.end()
          , std::bind(&chunk::is_my_ptr, std::placeholders::_1, p, total_chunk_size_)
          );

        assert("After all we found chunk whom pointer belongs to" && i != chunks_.end());

        dealloc_last_ = i - chunks_.begin();
    }

    chunks_[dealloc_last_].deallocate(p, block_size_);
}

}}}

