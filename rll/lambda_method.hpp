#pragma once

#include "method.hpp"
#include "agent.hpp"
#include "detail/utils.hpp"

#include <unordered_map>
#include <vector>
#include <utility>

namespace tcl { namespace rll {

/// @file Unified view for MC and TD methods.
/// Use eligibility traces.
/// Maintain distinct traces for every agent in system.

/// @brief On-policy TD(lambda) method for state value function.
/// Algorithm described in
/// http://webdocs.cs.ualberta.ca/~sutton/book/ebook/node75.html
class CLambdaTD : public CStateMethod 
{
public:
    CLambdaTD(CEnvState* i_pEnv, CConfigPtr i_ptrConfig);

private:
    typedef std::unordered_map<
        CVectorRlltPtr
      , double
      , detail::EvalVectorRlltPtrHash
      , detail::IsEqualVectorRlltPtr
      > CTraceMap;

    typedef std::vector<CTraceMap> CTraces;

private:
	/// @name CStateMethod implementation
	/// @{
    /// @brief Update value function for specific agent with new reward
    void updateValueFunctionImpl(CEnvState* i_env, int i_agentIndex, double i_reward);

    /// @brief Update value function for all agents because terminal state was reached
    void updateValueFunctionOnTerminalImpl(CEnvState* i_env, const CVectorDbl& i_rewards);
	/// @}

    void prepareUpdates(
        const CAgentPtr& i_ptrAgent
      , CTraceMap& io_agentTraces
      , const CVectorRlltPtr& i_stateForUpdate
      , double i_stateValue
      , double i_nextStateValue
      , double i_reward
      , CAgent::CUpdateList& o_updateList
      );

private:
    CTraces m_traces;       //!< Eligibility trace for each agent
    CConfigPtr m_ptrConfig;
};

/// @brief On-policy TD(lambda) method for state-action value function.
/// Algorithm described in
/// http://webdocs.cs.ualberta.ca/~sutton/book/ebook/node64.html
class CLambdaSarsa : public CActionMethod 
{
public:
    CLambdaSarsa(CEnvAction* i_pEnv, CConfigPtr i_ptrConfig) 
        : CActionMethod(i_pEnv, i_ptrConfig) 
    {
    }

protected:
    /// @brief Update value function for specific agent with new reward
    void updateValueFunctionImpl(int i_agentIndex, double i_reward);

    /// @brief Update value function for all agents cause terminal state was reached
    void updateValueFunctionOnTerminalImpl(const CVectorDbl& rewards);
};

/// @brief Off-policy TD(lambda) method for state-action value function.
/// Algorithm described in
/// http://webdocs.cs.ualberta.ca/~sutton/book/ebook/node78.html
class CLambdaWatkins : public CActionMethod 
{
public:
    CLambdaWatkins(CEnvAction* i_pEnv, CConfigPtr i_ptrConfig) 
        : CActionMethod(i_pEnv, i_ptrConfig) 
    {
    }

protected:
    /// @brief Update value function for specific agent with new reward
    void updateValueFunctionImpl(int i_agentIndex, double i_reward);

    /// @brief Update value function for all agents cause terminal state was reached
    void updateValueFunctionOnTerminalImpl(const CVectorDbl& rewards);
};

}}
