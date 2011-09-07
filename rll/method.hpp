#pragma once

#include "rll_fwd.hpp"
#include "policy.hpp"

#include <map>

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
    /// @brief Update value function for specific agent with new reward
    /// @param activeAgent - current active agent
    /// @param activeAgent - active agent index
    /// @param newState - new state of environment right immediatelly after agent selection.
    /// Can be null if this is post terminal state. Value function for post terminal state 
    /// is always 0.
    /// @param reward - reward that agent got by selecting @c newState
    virtual void updateValueFunctionImpl(
        const CAgentPtr& activeAgent
      , int activeAgentIdx
      , const CStatePtr& newState 
      , double reward
      ) = 0;

private:
    /// @brief Process episode as states method.
    void processEpisode(unsigned int i_episode);

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
    /// @brief Update value function for specific agent with new reward
    virtual void updateValueFunctionImpl(int i_agentIndex, double i_reward) = 0;

private:
    /// @brief Process episode as actions-states method.
    virtual void processEpisode(unsigned int i_episode);

    CEnvAction* m_pEnv;  //!< Environment
};

}}
