#ifndef __RLDREAM_ENVIRONMENT_H__
#define __RLDREAM_ENVIRONMENT_H__

#include "agent.hpp"

namespace tcl { namespace rll {

/// @brief Environment interfaces base.
/// You must inherit one of CEnvState or CEnvAction and override virtuals.
/// In constructor of your class you should set active agent to whatever you 
/// want and fill agents vector
class CEnvBase 
{
public:
    CEnvBase() : m_activeAgent(0) {}

protected:
    /// @name Methods that should be overrided by developer
    /// @{
    /// @brief Set members to initial state.
    /// Should set following members
    /// - State
    /// - Action(if required)
    /// - m_activeAgent(if required. By default it is 0)
    virtual void initEpisode() = 0;

    /// @brief Return true if current state is terminal state
    virtual bool isEpisodeFinished() = 0;

    /// @brief Assign reward for each agent after we get to new state.
    /// When process o_ptrRewards already has size equals to o_ptrRewards and
    /// every item initialized to 0.0
    virtual void observeRewards(CVectorDbl& o_rewards) = 0;

    /// @brief Set next active agent index.
    /// Active agent index always available as m_activeAgent member
    virtual void selectNextAgent() {}
    /// @}

    int m_activeAgent;               //!< Current active agent index
    std::vector<CAgentPtr> m_agents; //!< Agents vector

    /// @brief Current environment state.
    /// Must be overwrited only by internal classes. User MUST NOT change this 
    /// member during ovverrided methods calls.
    /// It is automatically changes after "fillPossibilities" and before 
    /// "observeRewards"
    CStatePtr m_ptrState;
    
    /// @brief Previous environment state.
    /// Maintained by Methods
    CStatePtr m_ptrPrevState;

    unsigned int m_episode; //!< Current episode
    unsigned int m_step;    //!< Current step in episode
};

/** @brief Environment for state value function */
class CEnvState : public CEnvBase {
public:

  friend class CStateMethod;
  friend class CLambdaTD;

  typedef std::vector<CStatePtr> CPossibleStates;

protected:
  /// @brief Get possible next states from current state
  virtual void fillPossibilities(CPossibleStates& o_states) = 0;

  /// @brief Value function cache.
  /// Contains already calculated value for m_ptrState and active agent.
  /// Can be helpfull in updateValueFunction implementation.
  double m_cache;
};

/// @brief Environment for state-action value function
class CEnvAction : public CEnvBase {
public:

  friend class CActionMethod;
  friend class CLambdaSarsa;
  friend class CLambdaWatkins;

  typedef std::vector<CActionPtr> CPossibleActions;

protected:
  /// @brief Get next state and possible actions from next state
  /// @return Next state according to m_ptrAction
  virtual void fillPossibilities(CPossibleActions& o_actions) = 0;

  /// @brief Return next state by previous state and performed action
  virtual CStatePtr getNextState(CStatePtr i_ptrState, CActionPtr i_ptrAction) = 0;
};

}}

#endif //__RLDREAM_ENVIRONMENT_H__
