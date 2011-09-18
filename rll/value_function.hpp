#pragma once

#include "rll_fwd.hpp"

#include <utility>
#include <vector>

namespace tcl { namespace rll {

/// @brief Interface for value function.
class value_function 
{
public:
    typedef std::vector<std::pair<vector_rllt_csp, double> > update_list;

    virtual ~value_function() {}

    /// @brief Return value for internal representation of state.
    virtual double get_value(const vector_rllt_csp& st) = 0;

    /// @brief Correct value function according update list
    /// Each entry represent 
    virtual void update(const update_list& lst) = 0;
};

}}
