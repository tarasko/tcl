#pragma once

#include "greedy.hpp"

namespace tcl { namespace rll { namespace policy {

/// @brief Epsilon greedy policy.
/// This policy allows to do exploration and explotation simultaniosly.
/// In most cases it will choose 
class egreedy : public iface
{
public:
    egreedy(double epsilon = 0.05);

    virtual const selection& select(const variants& sorted_variants);

private:
    greedy greedy_;
    double epsilon_;
};

}}}
