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
class CLookupTable : public CValueFunction 
{
public:
    CLookupTable(double i_init = 0.0) : m_init(i_init) {}

    /// @brief Return value for internal representation of state.
    virtual double GetValue(const CVectorRlltPtr& i_ptrState);
    /// @brief Correct value function according update map.
    /// @todo think about checkState for interger values
    virtual void Update(const CUpdateList& i_list);

private:

    typedef std::unordered_map<
        CVectorRlltPtr
      , double
      , detail::EvalVectorRlltPtrHash
      , detail::IsEqualVectorRlltPtr
      > CValueMap;
    
    CValueMap m_values;  //!< Map from data vector to value.
    double m_init;       //!< Initial value for new states.
};

}}
