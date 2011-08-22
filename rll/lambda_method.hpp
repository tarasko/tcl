#pragma once

#include "method.hpp"

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

protected:
    /// @brief Update value functions for previous states using last rewards.
    virtual CVectorRlltPtr updateValueFunction(
        int i_agentIndex
      , double i_reward
      , bool i_terminal
      );

    /// @brief Return previous state for agent.
    ///
    /// If agent already has previous internal state then return it. If not then
    /// take environment previous state. Change last member which identifies agent 
    /// to i_agent and return it. This is the way to make previous states for 
    /// initial states.
    ///
    /// @param i_agent - agent index
    CVectorRlltPtr getPreviousState(int i_agentIndex);
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
