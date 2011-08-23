#pragma once

#include "../rll_fwd.hpp"

#include <algorithm>

namespace tcl { namespace rll {
namespace detail {

template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

struct IsEqualVectorRlltPtr
{
    bool operator()(const CVectorRlltPtr& f, const CVectorRlltPtr& s) const
    {
        return std::equal(f->begin(), f->end(), s->begin());
    }
};

struct EvalVectorRlltPtrHash
{
    size_t operator()(const CVectorRlltPtr& v) const
    {
        size_t seed = 0;
        std::for_each(v->begin(), v->end(), [&seed](rll_type val) 
        {
            detail::hash_combine(seed, val);
        });
        return seed;
    }
};

/// @brief Translate from external state to internal.
///
/// If we use state value function pass null action pointer.
/// Active agent index append to vector as last element
CVectorRlltPtr translate(
    const CStatePtr& i_ptrState
  , const CActionPtr& i_ptrAction
  , int i_agent
  );

}
}}
