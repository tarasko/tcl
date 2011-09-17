#pragma once

#include "rll_fwd.hpp"
#include "agent.hpp"

namespace tcl { namespace rll {

namespace detail { class CMethodBase; }

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
    friend class CMethodBase;

public:
    // Agents accessors
    std::vector<CAgentPtr>& agents();
    const std::vector<CAgentPtr>& agents() const;

    // Return current step and episode
	unsigned int episode() const ;
	unsigned int step() const;

    /// @name Define the environment and must be implemented by user class.
    /// @{
    /// @brief Method will call this when new episode begins.
    virtual void initEpisode() = 0;

    /// @brief Return current active agent.
    virtual size_t activeAgent() const = 0;

    /// @brief Return current state.
    virtual CStatePtr currentState() const = 0;
    /// @}
    
private:
    std::vector<CAgentPtr> m_agents; //!< Agents vector
    const CMethodBase*     m_method; //!< Backref to method that works on environment
};

/// @brief Environment for state value function.
class CEnvState : public CEnvBase 
{
public:
    /// @brief Get possible next states for active agent for current environment state.
    virtual std::vector<CStatePtr> getPossibleNextStates() const = 0;

    /// @brief Set one of state returned from getPossibleNextStates as current, assign rewards to agents.
    /// To assign rewards to agents, call CAgent::addReward.
    /// This method also may change active agent.
    /// @return True if method should continue with next step, false if we got to terminal state.
    virtual bool setNextStateAssignRewards(const CStatePtr& state) = 0;
};

/// @brief Environment for state-action value function.
class CEnvAction : public CEnvBase
{
public:
    /// @brief Get possible actions for active agent for current environment state.
    virtual std::vector<CActionPtr> getPossibleActions() const = 0;

    /// @brief Active agent should take current action and recieve reward.
    /// @return True if method should continue with next step, false if we got to terminal state.
    virtual bool doActionAssignRewards(const CActionPtr& action) = 0;
};

}}
