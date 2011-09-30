#pragma once

#include "../rll_fwd.hpp"

#include <algorithm>

namespace tcl { namespace rll { namespace detail {

// Stolen from Boost.Hash
template <class T>
void hash_combine(std::size_t& seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

struct eval_vector_rllt_csp_hash
{
    size_t operator()(const vector_rllt_csp& v) const
    {
        size_t seed = 0;
        std::for_each(v->begin(), v->end(), [&seed](rll_type val) 
        {
            hash_combine(seed, val);
        });
        return seed;
    }
};

struct is_equal_vector_rllt_csp
{
    bool operator()(const vector_rllt_csp& f, const vector_rllt_csp& s) const
    {
        return std::equal(f->begin(), f->end(), s->begin());
    }
};

}}}
