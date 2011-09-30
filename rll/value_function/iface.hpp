#pragma once

#include "../rll_fwd.hpp"
#include "iface.hpp"

#include <utility>
#include <vector>

namespace tcl { namespace rll { namespace value_function {

/// @brief Interface for value function.
class iface
{
public:
    typedef std::vector<std::pair<vector_rllt_csp, double> > update_list;

    virtual ~iface() {}

    /// @brief Return value for internal representation of state.
    virtual double get_value(const vector_rllt_csp& st) = 0;

    /// @brief Correct value function according update list.
    virtual void update(const update_list& lst) = 0;
};

}}}
