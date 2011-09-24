#pragma once

#if !defined(TCL_RLL_INT)
#error "Lookup table is only available in integer version of RLL"
#endif

#include "iface.hpp"

#include "../detail/utils.hpp"

#include <unordered_map>

namespace tcl { namespace rll { namespace value_function {

/// @brief Implement value function based on lookup table.
/// Works only for state (and action) variables which has type int.
class lookup_table : public iface 
{
public:
    lookup_table(double init = 0.0);

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

inline lookup_table::lookup_table(double init) 
    : init_(init) 
{
}

}}}
