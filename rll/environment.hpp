#pragma once

#include "rll_fwd.hpp"

namespace tcl { namespace rll {

namespace detail { class method_base; }

/// @brief Environment base class.
/// Application must inherit one of env_state or env_action and override virtuals.
/// During construction you have to create agents and push them to agents vector.
class env_base 
{
    friend class method_base;

public:
    std::vector<agent_sp>& agents();
    const std::vector<agent_sp>& agents() const;

	unsigned int episode() const ;
	unsigned int step() const;

    /// @brief Method will call this when new episode begins.
    virtual void init_episode() = 0;

    /// @brief Must return current active agent index.
    virtual size_t active_agent() const = 0;
    
private:
    std::vector<agent_sp> agents_; //!< Agents vector
    const method_base*     method_; //!< Backref to method that works on environment
};

/// @brief Environment for state value function.
/// @copy env_base
class env_state : public env_base 
{
public:
    typedef state state_type;

    /// @brief Return current state.
    virtual state_type current_state() const = 0;

    /// @brief Get possible next states for active agent for current environment state.
    virtual std::vector<state_type> get_possible_next_states() const = 0;

    /// @brief Set one of state returned from get_possible_next_states as current, assign rewards to agents.
    /// To assign rewards to agents, call agent::add_reward.
    /// This method also may change active agent.
    /// @return True if method should continue with next step, false if we got to terminal state.
    virtual bool set_next_state_assign_rewards(const state_type& state) = 0;
};

/// @brief Environment for state-action value function.
/// @copy env_base
class env_action : public env_base
{
public:
    /// @brief State type suitable for this environment.
    typedef state_with_reserved_action state_type;

    /// @brief Return current state.
    virtual state_type current_state() const = 0;

    /// @brief Get possible actions for active agent for current environment state.
    virtual std::vector<rll_type> get_possible_actions() const = 0;

    /// @brief Active agent should take current action and recieve reward.
    /// @return True if method should continue with next step, false if we got to terminal state.
    virtual bool do_action_assign_rewards(rll_type action) = 0;
};

}}
