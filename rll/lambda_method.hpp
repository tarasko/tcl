#pragma once

#include "method.hpp"
#include "detail/utils.hpp"

#include <unordered_map>
#include <vector>
#include <utility>

namespace tcl { namespace rll {

/// @file Unified view for MC and TD methods.
/// Use eligibility traces.
/// Maintain distinct traces for every agent in system.

/// @brief TD(lambda) method for state value function.
class CLambdaTD : public CStateMethod 
{
public:
    CLambdaTD(CEnvState* i_pEnv, CConfigPtr i_ptrConfig) 
        : CStateMethod(i_pEnv, i_ptrConfig) 
    {
    }

private:
    typedef std::unordered_map<
        CVectorRlltPtr
      , double
      , detail::EvalVectorRlltPtrHash
      , detail::IsEqualVectorRlltPtr
      > CTraceMap;

    typedef std::vector<CTraceMap> CTraces;

    /// @brief Update value function for specific agent with new reward
    void updateValueFunctionImpl(int i_agentIndex, double i_reward);

    /// @brief Update value function for all agents cause terminal state was reached
    void updateValueFunctionOnTerminalImpl(const CVectorDbl& rewards);

    CTraces m_traces;
};

/// @brief Sarsa method for state value function.
class CLambdaSarsa : public CActionMethod 
{
public:
    CLambdaSarsa(CEnvAction* i_pEnv, CConfigPtr i_ptrConfig) 
        : CActionMethod(i_pEnv, i_ptrConfig) 
    {
    }

protected:
    /// @brief Update value functions for previous act.-st. using last rewards.
    virtual CVectorRlltPtr updateValueFunction(
        int i_agentIndex
      , double i_reward
      , bool i_terminal
      );
};

/// @brief Implements q-learning method with eligibility traces.
class CLambdaWatkins : public CActionMethod 
{
public:
    CLambdaWatkins(CEnvAction* i_pEnv, CConfigPtr i_ptrConfig) 
        : CActionMethod(i_pEnv, i_ptrConfig) 
    {
    }

protected:
    /// @brief Update value functions for previous act.-st. using last rewards.
    virtual CVectorRlltPtr updateValueFunction(
        int i_agentIndex
      , double i_reward
      , bool i_terminal
      );
};

}}
