#pragma once

#include "rll_fwd.hpp"
#include "detail/policy.hpp"

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
    CMethodBase(CEnvBase* pEnv, const CConfigPtr& ptrConfig);
    virtual ~CMethodBase();

    /// @brief Run training loop.
    /// @param episodes - Number of episodes to train
    void run(unsigned int episodes);

    unsigned int episode() const;
    unsigned int step() const;

protected:
    /// @brief Run episode single episode
    virtual void runEpisode() = 0;  

    CConfigPtr      m_ptrConfig;  //!< Config information
    detail::CPolicy m_policy;     //!< Policy
    CEnvBase*       m_pEnv;       //!< Environment

    unsigned int    m_episode;    //!< Current episode
    unsigned int    m_step;       //!< Current step in episode
};

/// @brief Base class for methods which operate only on states
class CStateMethod : public CMethodBase 
{
public:
    CStateMethod(CEnvState* pEnv, const CConfigPtr& ptrConfig);

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
    void runEpisode();
};

/// @brief Base class for methods which operate on state-action pairs
class CActionMethod : public CMethodBase 
{
public:
    CActionMethod(CEnvAction* i_pEnv, const CConfigPtr& i_ptrConfig);

    void run(unsigned int episodes);

    using CMethodBase::episode;
    using CMethodBase::step;

protected:
    /// @brief Update value function for specific agent with new reward
    virtual void updateValueFunctionImpl(int i_agentIndex, double i_reward) = 0;

private:
    /// @brief Process episode as actions-states method.
    void runEpisode();
};

}}
