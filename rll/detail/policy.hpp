#pragma once

#include "../rll_fwd.hpp"

#include <random>

namespace tcl { namespace rll { namespace detail {

/// @brief Policy interface.
/// Greedy, e-greedy and softmax policies should implement this interface. 
class policy
{
public:
    // Estimated value -> state (or state-action)
    typedef std::pair<double, vector_rllt_sp> selection;
    typedef std::vector<selection> variants;

    virtual ~policy() = 0 {}

    /// @brief Select action(or state) from variants.
    virtual const selection& select(const variants& sorted_variants) = 0;

protected:
    std::mt19937 gen_; //!< Random number generator
};

}}}
