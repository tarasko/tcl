#pragma once

#include "rll_fwd.hpp"
#include "state.hpp"

#include "policy/iface.hpp"

namespace tcl { namespace rll {

/// @brief Base class for RL methods.
class method_base 
{
public:
    method_base(env_base* env, policy::iface* policy, const CConfigPtr& config);
    virtual ~method_base();

    /// @brief Run training loop.
    /// @param episodes - Number of episodes to train
    void run(unsigned int episodes);

    unsigned int episode() const;
    unsigned int step() const;

protected:
    /// @brief Run single episode.
    virtual void run_episode_impl() = 0;  

    CConfigPtr      config_;     //!< Config information
    policy::iface*  policy_;     //!< Policy
    env_base*       env_;        //!< Environment

    unsigned int    episode_;    //!< Current episode
    unsigned int    step_;       //!< Current step in episode
};

/// @brief Base class for methods which operate only on states
class method_state : public method_base 
{
public:
    typedef state state_type;

    method_state(env_state* env, policy::iface* policy, const CConfigPtr& config);

protected:
    /// @brief Update value function for specific agent with new reward
    /// @param active_agent - current active agent
    /// @param active_agent - active agent index
    /// @param new_state_value - estimated value for new state returned by value function
    /// When next state is terminal it should be 0.0
    /// @param reward - reward that agent got by selecting @c new state
    virtual void update_value_function_impl(
        const agent_sp& active_agent
      , int active_agent_idx
      , double policy_selection_value
      , double greedy_selection_value
      , double reward
      ) = 0;

    /// @brief Process episode as states method.
    virtual void run_episode_impl();

private:
    typedef policy::iface::variants value_state_map;

    /// Used internally by run_episode_impl
    /// Makeing this as member helps to avoid allocations for next 
    /// possible state on each step.
    value_state_map variants_; 
};

/// @brief Base class for methods which operate on state-action pairs
class method_action : public method_base 
{
public:
    typedef state_with_reserved_action state_type;

    method_action(env_action* env, policy::iface* policy, const CConfigPtr& config);

protected:
    /// @brief Update value function for specific agent with new reward
    virtual void update_value_function_impl(
        const agent_sp& active_agent
      , int active_agent_idx
      , double policy_selection_value
      , double greedy_selection_value
      , double reward
      ) = 0;

    /// @brief Process episode as actions-states method.
    virtual void run_episode_impl();

private:
    typedef policy::iface::variants value_action_map;

    /// Used internally by run_episode_impl
    /// Makeing this as member helps to avoid allocations for next 
    /// possible state on each step.
    value_action_map variants_; 
};

}}
