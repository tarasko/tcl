#pragma once

#include "types.hpp"
#include "config.hpp"
#include "policy.hpp"
#include "environment.hpp"

#include <memory>
#include <list>

namespace tcl { namespace rll {

/// @brief Base class for RL methods.
///
/// @todo Every method likes to repeat some operations for one state. Generally 
/// I mean state value calculation both in SelectNext and Reward methods. Find 
/// such places and make some kind of cache variables.
///
/// @todo Hide policy from include files
class CMethodBase 
{
public:
    CMethodBase(const CConfigPtr& i_ptrConfig) 
        : m_ptrConfig(i_ptrConfig)
        , m_policy(i_ptrConfig) 
    {
    }

    /// @brief Run training loop 
    /// @param i_episodes - Number of episodes to train
    void Run(unsigned int i_episodes);

protected:
    /// @brief Process episode depend on method type
    virtual void processEpisode(unsigned int i_episode) = 0;  

    CConfigPtr m_ptrConfig;  //!< Config information
    CPolicy m_policy;        //!< Policy
};

/// @brief Base class for methods which operate only on states
class CStateMethod : public CMethodBase 
{
public:
    CStateMethod(CEnvState* i_pEnv, const CConfigPtr& i_ptrConfig) 
        : CMethodBase(i_ptrConfig), m_pEnv(i_pEnv) 
    {
    }

    typedef std::multimap<double, CStatePtr> CValueStateMap;

protected:
    /// @brief Process episode as states method.
    virtual void processEpisode(unsigned int i_episode);

    /// @brief Update value functions for previous states using last rewards.
    virtual CVectorRlltPtr updateValueFunction(
        int i_agentIndex
      , double i_reward
      , bool i_terminal
      ) = 0;

    CEnvState* m_pEnv;  //!< Environment
};

/// @brief Base class for methods which operate on state-action pairs
class CActionMethod : public CMethodBase 
{
public:
    CActionMethod(CEnvAction* i_pEnv, const CConfigPtr& i_ptrConfig) 
        : CMethodBase(i_ptrConfig)
        , m_pEnv(i_pEnv) 
    {
    }

    typedef std::multimap<double, CActionPtr> CValueActionMap;

protected:
    /// @brief Process episode as actions-states method.
    /// It guaranties that updateValueFunction with terminal flag as 'true' will 
    /// never be called for first agent action.
    virtual void processEpisode(unsigned int i_episode);

    /// @brief Update value functions for previous act.-st. using last rewards.
    virtual CVectorRlltPtr updateValueFunction(
        int i_agentIndex
      , double i_reward
      , bool i_terminal
      ) = 0;

private:
    void performAgentUpdate(const CVectorDbl& i_rewards);

protected:
    CEnvAction* m_pEnv;               //!< Environment
    CActionPtr m_ptrPerformedAction;  //!< Performed action
    double m_performedValue;          //!< Performed action value
    CActionPtr m_ptrGreedyAction;     //!< Greedy action
    double m_greedyValue;             //!< Greedy action value
};

}}
