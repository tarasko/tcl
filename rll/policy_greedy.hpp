#pragma once

#include "detail/policy.hpp"

namespace tcl { namespace rll {

/// @brief Simple greedy policy.
/// Always select variant that has maximal value.
/// If few variants have same maximal value then one them will be choosen randomly.
class policy_greedy : public detail::policy
{
public:
    virtual const selection& select(const variants& sorted_variants);

protected:
    const selection& select_with_hint(
        const variants& sorted_variants
      , variants::const_iterator lb_hint
      );
};

}}
