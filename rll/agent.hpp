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

    CStatePtr lastStateWhenWasActive() const;
    void setLastStateWhenWasActive(const CStatePtr& state);
    CActionPtr lastActionWhenWasActive() const;
    void setLastActionWhenWasActive(const CActionPtr& action);

    void addReward(double reward);
    double releaseReward();

private:
    CValueFunctionPtr m_ptrFunc;   //!< Value function for agent

    CStatePtr  m_state;
    CActionPtr m_action;

    double m_reward;
};

inline CStatePtr CAgent::lastStateWhenWasActive() const
{
    return m_state;
}

inline void CAgent::setLastStateWhenWasActive(const CStatePtr& state)
{
    m_state = state;
}

inline CActionPtr CAgent::lastActionWhenWasActive() const
{
    return m_action;
}

inline void CAgent::setLastActionWhenWasActive(const CActionPtr& action)
{
    m_action = action;
}

}}
