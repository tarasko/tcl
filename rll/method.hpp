#pragma once

#include "rll_fwd.hpp"
#include "state.hpp"
#include "detail/policy.hpp"

namespace tcl { namespace rll {

/// @brief Base class for RL methods.
///
/// @todo Every method likes to repeat some operations for one state. Generally 
/// I mean state value calculation both in SelectNext and Reward methods. Find 
/// such places and make some kind of cache variables.
///
/// @todo Hide policy from include files.
class method_base 
{
public:
    method_base(env_base* env, const CConfigPtr& config);
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
    detail::CPolicy policy_;     //!< Policy
    env_base*       env_;        //!< Environment

    unsigned int    episode_;    //!< Current episode
    unsigned int    step_;       //!< Current step in episode
};

/// @brief Base class for methods which operate only on states
class method_state : public method_base 
{
public:
    typedef state state_type;

    method_state(env_state* env, const CConfigPtr& config);

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
      , double new_state_value
      , double reward
      ) = 0;

    /// @brief Process episode as states method.
    virtual void run_episode_impl();

private:
    typedef std::vector<std::pair<double, state_type> > value_state_map;

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

    method_action(env_action* env, const CConfigPtr& config);

protected:
    /// @brief Update value function for specific agent with new reward
    virtual void update_value_function_impl(
        const agent_sp& active_agent
      , int active_agent_idx
      , const std::pair<double, vector_rllt_csp>& policy_selection
      , const std::pair<double, vector_rllt_csp>& greedy_selection
      , double reward
      ) = 0;

    /// @brief Process episode as actions-states method.
    virtual void run_episode_impl();

private:
    typedef std::vector<std::pair<double, vector_rllt_csp> > value_action_map;

    /// Used internally by run_episode_impl
    /// Makeing this as member helps to avoid allocations for next 
    /// possible state on each step.
    value_action_map variants_; 
};

}}
