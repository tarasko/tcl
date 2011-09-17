#pragma once

#include "method.hpp"
#include "agent.hpp"
#include "detail/utils.hpp"

#include <unordered_map>
#include <vector>
#include <utility>

namespace tcl { namespace rll {

template<typename Base>
class COnPolicyLambdaUpdater : public Base
{
public:
    template<typename EnvType> 
    COnPolicyLambdaUpdater(EnvType* env, const CConfigPtr& config)
        : Base(env, config)
        , m_traces(env->agents().size())
    {
    }

    void updateValueFunctionHelper(
        const CAgentPtr& agent
      , size_t agentIdx
      , const CVectorRlltPtr& oldState
      , double newStateValue
      , double reward
      );

private:
    typedef std::unordered_map<
        CVectorRlltPtr
      , double
      , detail::EvalVectorRlltPtrHash
      , detail::IsEqualVectorRlltPtr
      > CTraceMap;

    typedef std::vector<CTraceMap> CTraces;

    CTraces m_traces;
};

class COffPolicyLambdaUpdater
{
};

/// @file Unified view for MC and TD methods.
/// Use eligibility traces.
/// Maintain distinct traces for every agent in system.

/// @brief On-policy TD(lambda) method for state value function.
/// Algorithm described in
/// http://webdocs.cs.ualberta.ca/~sutton/book/ebook/node75.html
class CLambdaTD : public COnPolicyLambdaUpdater<CStateMethod>
{
public:
    CLambdaTD(CEnvState* env, const CConfigPtr& config);

private:
	/// @name CStateMethod implementation
	/// @{
    /// @brief Update value function for specific agent with new reward
    void updateValueFunctionImpl(
        const CAgentPtr& activeAgent
      , int activeAgentIdx
      , const std::pair<double, CStatePtr>& newStateWithValue
      , double reward
      );
    /// @}
};

/// @brief On-policy TD(lambda) method for state-action value function.
/// Algorithm described in
/// http://webdocs.cs.ualberta.ca/~sutton/book/ebook/node64.html
class CLambdaSarsa : public COnPolicyLambdaUpdater<CActionMethod> 
{
public:
    CLambdaSarsa(CEnvAction* pEnv, const CConfigPtr& ptrConfig);

protected:
    /// @brief Update value function for specific agent with new reward
    void updateValueFunctionImpl(
        const CAgentPtr& activeAgent
      , int activeAgentIdx
      , const std::pair<double, CActionPtr>& policySelection
      , const std::pair<double, CActionPtr>& greedySelection
      , double reward
      );
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

};

}}
