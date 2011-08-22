#pragma once

#include "rll_fwd.hpp"

#include <string>
#include <map>

#pragma warning(disable : 4290)

namespace tcl { namespace rll {

/** @brief Represent state signal.
 Every variable in state signal should have either int or double type.
 OTHERWISE YOU CAN GET bad_any_cast EXCEPTION!!!. Some implementation of 
 value function requires int type for all variables because it must have 
 exact sense when one state is equal to another. This is not possible 
 if some variable is double. Actually double variable is a problem for 
 feeling equal states. 
 @todo Replace strings to string refs */
class CState 
{
public:
    DEFINE_EXCEPTION(CStateError);

    /** @brief Register named variable to state signal */
    void RegisterVariable(const std::string& i_name);
    /** @brief Destroy named variable in state signal */
    void RevokeVariable(const std::string& i_name);

    /** @brief Set value for named variable */
    void SetValue(const std::string& i_name, rll_type i_val);
    /** @brief Return value for named variable */
    rll_type GetValue(const std::string& i_name);

    /** @brief Return state variables as array */
    CVectorRlltPtr GetData() const;

    /** @brief Spawn new state the exact copy of exist one */
    CStatePtr Clone() const;
    /** @brief Compare state with another one */
    bool IsEqual(const CStatePtr& i_ptrState) const;

protected:
    typedef std::map<std::string, rll_type> CVarMap;
    CVarMap m_vars;  //!< Map of named variables
};

}}
