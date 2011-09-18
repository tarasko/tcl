#pragma once

#if !defined(TCL_RLL_INT)
#error "Lookup table is only available in integer version of RLL"
#endif

#include "rll_fwd.hpp"
#include "value_function.hpp"

#include "detail/utils.hpp"

#include <unordered_map>
#include <algorithm>

namespace tcl { namespace rll {

/// @brief Implement value function based on lookup table.
/// Works only for state (and action) variables which has type int.
class vf_lookup_table : public value_function 
{
public:
    vf_lookup_table(double init = 0.0) : init_(init) {}

    /// @brief Return value for internal representation of state.
    virtual double get_value(const vector_rllt_csp& st);

    /// @brief Correct value function according update map.
    /// @todo think about checkState for interger values
    virtual void update(const update_list& lst);

private:

    typedef std::unordered_map<
        vector_rllt_csp
      , double
      , detail::eval_vector_rllt_csp_hash
      , detail::is_equal_vector_rllt_csp
      > value_map;
    
    value_map values_;  //!< Map from data vector to value.
    double init_;       //!< Initial value for new states.
};

}}
