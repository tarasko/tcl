#include "fixed_pool.hpp"

#include <iostream>

namespace tcl { namespace allocators {

fixed_pool::fixed_pool(size_type chunks_num, size_t chunk_size)
    : chunks_num_(chunks_num)
    , chunk_size_(chunk_size > sizeof(size_type) ? chunk_size : sizeof(size_type))
    , total_size_(chunk_size_ * chunks_num_)
    , chunks_(new unsigned char[total_size_])
{
    unsigned char* p = chunks_.get();
    size_type i = 0;

    for(; i < chunks_num_; p += chunk_size_)
        *reinterpret_cast<size_type*>(p) = ++i;

    chunk_ref new_head;
    new_head.idx_ = 0;
    new_head.generation_ = 0;
    head_.store(new_head, boost::memory_order_relaxed);
}

void* fixed_pool::allocate()
{
    chunk_ref old_head = head_.load(boost::memory_order_relaxed);
    chunk_ref new_head;

    void* res;

    do {
        if (old_head.idx_ == chunks_num_)
            return 0;

        res = chunks_.get() + chunk_size_ * old_head.idx_;
        new_head.idx_ = *reinterpret_cast<size_type*>(res);
        new_head.generation_ = old_head.generation_ + 1;
    }
    while(!head_.compare_exchange_weak(old_head, new_head));

    return res;
}

void fixed_pool::deallocate(void* p)
{
    assert("Ensure that p doesn`t violate lower bound" && (void*)p >= chunks_.get());

    chunk_ref old_head = head_.load(boost::memory_order_relaxed);
    chunk_ref new_head;

    size_type& new_idx = *reinterpret_cast<size_type*>(p);

    do {
        new_idx = old_head.idx_;

        new_head.idx_ = (reinterpret_cast<unsigned char*>(p) - chunks_.get()) / chunk_size_;
        new_head.generation_ = old_head.generation_ + 1;
    }
    while(!head_.compare_exchange_weak(old_head, new_head));
}

}}
