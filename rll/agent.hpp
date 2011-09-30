#pragma once

#include "rll_fwd.hpp"
#include "value_function/iface.hpp"

#include <cassert>

namespace tcl { namespace rll {

/// @brief Agent that participate in reinforcement learning. 
/// Agent ties together value function, policy, and (on/off)policy property.
/// The last one specify what value function is going to approximate. 
/// When onpolicy is specified then value function is approximated for 
/// specified policy. In case of off-policy, value function is approximated 
/// for greedy policy, it doesn`t depend what real policy you are going to use.
/// However the real policy still continue to be used to make select actions.
/// More about (on/off)policy is here
/// http://webdocs.cs.ualberta.ca/~sutton/book/ebook/node54.html
/// http://webdocs.cs.ualberta.ca/~sutton/book/ebook/node56.html
/// http://webdocs.cs.ualberta.ca/~sutton/book/ebook/node77.html
/// http://webdocs.cs.ualberta.ca/~sutton/book/ebook/node78.html
/// @n
/// You are supposed to create agents on environment startup.
/// 
/// @todo Hide prev_state, set_prev_state and release_reward from 
/// library users.
class agent 
{
public:
	typedef value_function::iface::update_list update_list;

    enum OnpolicyProperty
    {
        OFFPOLICY = 0,
        ONPOLICY = 1
    };

public:
    agent(
        value_function::iface* func
      , policy::iface* policy
      , OnpolicyProperty onpolicy = ONPOLICY
      );

    /// @brief Reward agent.
    /// Use this function to give reward to agent when method invokes 
    /// @c set_next_state_assign_rewards or do_action_assign_rewards.
    void add_reward(double reward);

    /// @brief Get sum of all rewards that was given since previous call.
    /// This method is for internal purposes, however you can use it to 
    /// reset reward to 0.0.
    double release_reward();

    /// @brief Return non-constant reference to value function.
    value_function::iface& vf();
    
    /// @brief Return constant reference to value function.
    const value_function::iface& vf() const;

    /// @brief Return constant reference to policy.
    policy::iface& policy();

    /// @brief Return true if on-policy property is enabled.
    bool onpolicy() const;

    /// @brief Return serialized representation for previous agent state.
    vector_rllt_csp prev_state() const;
    
    /// @brief Set previous agent state.
    void set_prev_state(const vector_rllt_csp& prev_state);

private:
    value_function::iface* func_;  //!< Value function for agent.
    policy::iface* policy_;        //!< Policy.
    OnpolicyProperty onpolicy_;    //!< Onpolicy property.
    vector_rllt_csp prev_state_;   //!< Agent previous state when was active.
    double reward_;                //!< Accumulated agent rewards since last time he was active.
};

inline agent::agent(
    value_function::iface* func
  , policy::iface* policy
  , OnpolicyProperty onpolicy
  ) 
  : func_(func)
  , policy_(policy)
  , onpolicy_(onpolicy)
  , reward_(0.0)
{
    assert(func_);
    assert(policy_);
}

inline void agent::add_reward(double reward)
{
    reward_ += reward;
}

inline double agent::release_reward()
{
    double tmp = reward_;
    reward_ = 0.0;
    return tmp;
}

inline value_function::iface& agent::vf()
{
    return *func_;
}
    
inline const value_function::iface& agent::vf() const
{
    return *func_;
}

inline policy::iface& agent::policy()
{
    return *policy_;
}

inline bool agent::onpolicy() const
{
    return !!onpolicy_;
}

inline vector_rllt_csp agent::prev_state() const
{
    return prev_state_;
}

inline void agent::set_prev_state(const vector_rllt_csp& prev_state)
{
    prev_state_ = prev_state;
}

}}
