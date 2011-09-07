#pragma once

#include "rll_fwd.hpp"
#include "agent.hpp"

#include <functional>

namespace tcl { namespace rll {

/// @brief Environment base class.
/// 
/// You must inherit one of CEnvState or CEnvAction and override virtuals.
/// In constructor of your class you should set active agent to whatever you 
/// want and fill agents vector.
/// Reinforcement learning method will run a specified number of episodes. 
/// Environment contains fixed number of agents. Each episode consist of variable 
/// number of steps. An episode finishes when we get to terminal state. Terminal 
/// state is signaled by returning true from observeReward method. 
class CEnvBase 
{
public:
    CEnvBase();

    // Agents accessors
    std::vector<CAgentPtr>& agents();
    const std::vector<CAgentPtr>& agents() const;

    /// @brief Method will call this when new episode begins.
    void initEpisode();

    /// @brief Return current active agent.
    /// Usually active agent is switched inside of nextStep.
    size_t activeAgent() const;

    /// @brief Return current state.
    CStatePtr currentState() const;

    /// @brief Method will call this if it ready to do the next step.
    /// Return true if environment successfully switched to new step.
    /// (Environment can change active agent here for example).
    /// Return false if we are already in terminal state. In this case method will
    /// observe terminal reward, do last update and init new episode.
    bool nextStep();

    /// @brief Return terminal rewards for each agents.
    CVectorDbl observeTerminalRewards() const;
    
	/// @brief Return current episode.
	unsigned int episode() const;

	/// @brief Return current step.
	unsigned int step() const;

protected:
    /// @name Methods that should be overriden by user
    /// @{
    /// @brief Called when episode begins.
    virtual void initEpisodeImpl() = 0;

    /// @brief Return current active agent.
    virtual size_t activeAgentImpl() const = 0;

    /// @brief Return current state.
    virtual CStatePtr currentStateImpl() const = 0;

    /// @brief Called when we ready to proceed with next step.
    /// @return false if current step is terminal and method was unable to 
    /// switch to next step.
    virtual bool nextStepImpl() = 0;

    /// @brief Return terminal rewards for each agents.
    /// Terminal reward for last active agent is ignored, because it was
    /// taken in account after last observeRewardImpl call
    virtual CVectorDbl observeTerminalRewardsImpl() const = 0;
    /// @}
    
private:
    std::vector<CAgentPtr> m_agents; //!< Agents vector

    unsigned int m_episode;          //!< Current episode
	unsigned int m_step;			 //!< Current step in episode
};

/// @brief Environment for state value function.
class CEnvState : public CEnvBase 
{
public:
    /// @brief Get possible next states for active agent for current environment state.
    virtual std::vector<CStatePtr> getPossibleNextStatesImpl() const = 0;

    /// @brief Set one of state returned from getPossibleNextStates as current and observe reward.
    virtual double setNextStateObserveRewardImpl(const CStatePtr& state) = 0;
};

/// @brief Environment for state-action value function.
class CEnvAction : public CEnvBase
{
public:
    /// @brief Get possible actions for active agent for current environment state.
    virtual std::vector<CActionPtr> getPossibleActionsImpl() const = 0;

    /// @brief Set one of state returned from getPossibleNextStates as current and observe reward.
    virtual double doActionObserveRewardImpl(const CActionPtr& action) const = 0;
};

inline CEnvBase::CEnvBase() : m_episode(unsigned int(-1))
{
}

inline CVectorDbl CEnvBase::observeTerminalRewards() const 
{
    return observeTerminalRewardsImpl();
}

inline size_t CEnvBase::activeAgent() const
{
    return activeAgentImpl();
}

inline CStatePtr CEnvBase::currentState() const
{
    return currentStateImpl();
}

inline std::vector<CAgentPtr>& CEnvBase::agents()
{
    return m_agents;
}

inline const std::vector<CAgentPtr>& CEnvBase::agents() const
{
    return m_agents;
}

inline unsigned int CEnvBase::episode() const
{
	return m_episode;
}

inline unsigned int CEnvBase::step() const
{
	return m_step;
}

}}
