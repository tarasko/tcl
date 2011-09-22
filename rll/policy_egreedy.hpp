#pragma once

#include "policy_greedy.hpp"

namespace tcl { namespace rll {

/// @brief Epsilon greedy policy.
/// This policy allows to do exploration and explotation simultaniosly.
/// In most cases it will choose 
class policy_egreedy : protected policy_greedy
{
public:
    policy_egreedy(double epsilon = 0.05);

    virtual const selection& select(const variants& sorted_variants);

private:
    double epsilon_;
};

}}
