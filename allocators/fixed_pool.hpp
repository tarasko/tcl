#pragma once

#include <boost/atomic.hpp>
#include <boost/cstdint.hpp>

#include <cassert>
#include <memory>

namespace tcl { namespace allocators {

/// \brief Lock free pool of memory chunks for object of specified type.
/// Both \c allocate and \c deallocate have O(1) complexity.
class fixed_pool
{
public:
    // this typedefs control ability of chunk_ref to fit
    // to 8 bytes limits.
    typedef boost::uint16_t size_type;
    typedef boost::uint16_t generation_type;

    typedef boost::int16_t difference_type;

    /// Construct pool with fixed number of chunks
    fixed_pool(size_type chunks_num, size_t chunk_size);

    void* allocate();
    void deallocate(void* p);

    size_t chunk_size() const
    {
        return chunk_size_;
    }

    bool is_my_ptr(void* p) const
    {
        // TODO: Optimize
        return p >= chunks_.get() && p <= chunks_.get() + chunk_size_ * chunks_num_;
    }

private:
    friend void intrusive_ptr_add_ref(fixed_pool* p)
    {
        ++p->ref_count_;
    }

    friend void intrusive_ptr_release(fixed_pool* p)
    {
        if (p->ref_count_.fetch_sub(1) == 1)
            delete p;
    }

    // noncopyable, nonassignable, no move support   
    fixed_pool(const fixed_pool&);
    fixed_pool(fixed_pool&& other);
    fixed_pool& operator=(const fixed_pool&);
    fixed_pool& operator=(fixed_pool&&);

    struct chunk_ref
    {
        size_type       idx_;        //!< Index of first available chunk
        generation_type generation_; //!< Help to resolve ABA problem.
    };

    const size_type chunks_num_;              //!< Number of chunks in \c chunks_
    const size_t chunk_size_;                 //!< Each chunk size
    std::unique_ptr<unsigned char[]> chunks_; //!< Memory block with implicit chunks

    boost::atomic<chunk_ref> head_;           //!< Index of first free chunk with generation number
    boost::atomic_int ref_count_;             //!< Reference counter for boost::intrusive_ptr
};

}}
