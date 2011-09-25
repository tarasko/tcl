#pragma once

#include "../rll_fwd.hpp"

namespace tcl { namespace rll {
namespace detail {

struct eval_vector_rllt_csp_hash
{
    size_t operator()(const vector_rllt_csp& v) const;
};

struct is_equal_vector_rllt_csp
{
    bool operator()(const vector_rllt_csp& f, const vector_rllt_csp& s) const;
};

template<bool SelectFirst>
inline double select_first_or_second(double f, double s)
{
    return f;
}

template<>
inline double select_first_or_second<false>(double f, double s)
{
    return s;
}

}
}}
