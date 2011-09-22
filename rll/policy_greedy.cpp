#include "policy_greedy.hpp"

#include <cassert>
#include <algorithm>

namespace tcl { namespace rll {

const policy_greedy::selection& policy_greedy::select(const variants& sorted_variants)
{
    assert(!sorted_variants.empty());

    auto lb_hint = std::lower_bound(
        sorted_variants.begin()
        , sorted_variants.end()
        , sorted_variants.back()
        , [](const selection& r1, const selection& r2) -> bool
        {
            return r1.first < r2.first;
        }
        );

    return select_with_hint(sorted_variants, lb_hint);            
}

const policy_greedy::selection& policy_greedy::select_with_hint(
    const variants& sorted_variants
    , variants::const_iterator lb_hint
    )
{
    std::uniform_int_distribution<> int_dist(
        lb_hint - sorted_variants.begin()
        , sorted_variants.size() - 1
        );

    return sorted_variants[int_dist(gen_)];
}

}}
