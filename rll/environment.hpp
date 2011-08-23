#pragma once

#include "rll_fwd.hpp"
#include "agent.hpp"

#include <functional>

namespace tcl { namespace rll {

/// @brief Environment interfaces base.
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

    /// @brief Set members to initial state.
    void initEpisode();

    /// @brief Assign reward for current agent and current state
    /// When process o_ptrRewards already has size equals to o_ptrRewards and
    /// every item initialized to 0.0
    /// @return Must return true if we got to terminal state.
    bool observeReward(double& o_reward) const;

    /// @brief Return terminal rewards for each agents.
    /// This method will be called after observeReward has returned true. Terminal reward.
    CVectorDbl observeTerminalRewards() const;

    /// @brief Set next active agent and return it index.
    int selectNextAgent();
    /// @}
    
    // Agents accessors
    std::vector<CAgentPtr>& agents();
    const std::vector<CAgentPtr>& agents() const;

    /// @brief Set current state.
    void setCurrentState(const CStatePtr& state);

    /// @brief Get current state.
    CStatePtr currentState() const;

    /// @brief Get previous state, return 0 if there is no previous state.
    CStatePtr previousState() const;

protected:

    /// @name Methods that should be overrided by user
    /// @{

    /// @brief Set members to initial state.
    /// Should set following members
    /// - State
    /// - Action(if required)
    /// - m_activeAgent(if required. By default it is 0)
    virtual void initEpisodeImpl() = 0;

    /// @brief Assign reward for each agent after we get to new state.
    /// When process o_ptrRewards already has size equals to o_ptrRewards and
    /// every item initialized to 0.0
    /// @return Must return true if we got to terminal state.
    virtual bool observeRewardImpl(double& o_reward) const = 0;

    /// @brief Return terminal rewards for each agents.
    /// This method will be called after observeReward has returned true. Terminal reward.
    virtual CVectorDbl observeTerminalRewardsImpl() const = 0;

    /// @brief Set next active agent and return it index.
    virtual int selectNextAgentImpl() = 0;
    /// @}
    
private:
    std::vector<CAgentPtr> m_agents; //!< Agents vector

    CStatePtr m_ptrState;            //!< Current environment state
    CStatePtr m_ptrPrevState;        //!< Previous environment state

    unsigned int m_episode;          //!< Current episode
};

/// @brief Environment for state value function.
class CEnvState : public CEnvBase 
{
public:
    typedef std::vector<CStatePtr> CPossibleStates;

    /// @brief Get possible next states from current state
    virtual void fillPossibilities(CPossibleStates& o_states) = 0;

    /// @brief Value function cache.
    /// Contains already calculated value for m_ptrState and active agent.
    /// Can be helpfull in updateValueFunction implementation.
    // double m_cache;
};

/// @brief Environment for state-action value function.
class CEnvAction : public CEnvBase
{
public:
    typedef std::vector<CActionPtr> CPossibleActions;

    /// @brief Get next state and possible actions from next state
    /// @return Next state according to m_ptrAction
    virtual void fillPossibilities(CPossibleActions& o_actions) = 0;

    /// @brief Return next state by previous state and performed action
    virtual void doAction(const CActionPtr& i_ptrAction) = 0;
};

inline CEnvBase::CEnvBase() : m_episode(unsigned int(-1))
{
}

inline void CEnvBase::initEpisode()
{
    m_ptrState.reset();
    m_ptrPrevState.reset();

    ++m_episode;

    std::for_each(m_agents.begin(), m_agents.end(), std::mem_fn(&CAgent::clean));

    initEpisodeImpl();
}

inline bool CEnvBase::observeReward(double& o_reward) const
{
    return observeRewardImpl(o_reward);
}

inline CVectorDbl CEnvBase::observeTerminalRewards() const 
{
    return observeTerminalRewardsImpl();
}

inline int CEnvBase::selectNextAgent()
{
    return selectNextAgentImpl();
}
    
inline std::vector<CAgentPtr>& CEnvBase::agents()
{
    return m_agents;
}

inline const std::vector<CAgentPtr>& CEnvBase::agents() const
{
    return m_agents;
}

inline void CEnvBase::setCurrentState(const CStatePtr& state)
{
    m_ptrPrevState = m_ptrState;
    m_ptrState = state;
}

inline CStatePtr CEnvBase::currentState() const
{
    return m_ptrState;
}

inline CStatePtr CEnvBase::previousState() const
{
    return m_ptrPrevState;
}

}}
