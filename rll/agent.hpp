#pragma once

#include "rll_fwd.hpp"
#include "value_function.hpp"

#include <list>
#include <memory>

namespace tcl { namespace rll {

/// @brief Helper struct for agents.
///
/// Contain various helpful information about agent.
/// Agent internal state differs from internal state stored in Method.
/// Additionaly it contains agent index on the bottom.
class CAgent 
{
public:
	typedef CValueFunction::CUpdateList CUpdateList;

public:
    CAgent(const CValueFunctionPtr& i_ptrFunc);

    /// @brief Return value for internal state representation.
    double getValue(const CVectorRlltPtr& state);
    /// @brief Correct value function according update map.
    void update(const CUpdateList& i_list);

private:
    CVectorRlltPtr m_ptrPrevState; //!< Last agent state when it was active
    CValueFunctionPtr m_ptrFunc;   //!< Value function for agent
};

}}
