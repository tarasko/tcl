#pragma once

#include "method.hpp"

#include "detail/utils.hpp"

#include <unordered_map>
#include <vector>
#include <utility>

namespace tcl { namespace rll {

template<typename Base>
class onpolicy_lambda_updater : public Base
{
public:
    template<typename EnvType> 
    onpolicy_lambda_updater(EnvType* env, policy::iface* policy, const CConfigPtr& config)
        : Base(env, policy, config)
        , traces_(env->agents().size())
    {
    }

    void update_value_function_helper(
        const agent_sp& active_agent
      , size_t active_agent_idx
      , const vector_rllt_csp& old_state
      , double new_state_value
      , double reward
      );

private:
    /// @brief Reset agent traces, call underlying run_episode_impl
    virtual void run_episode_impl();

    typedef std::unordered_map<
        vector_rllt_csp
      , double
      , detail::eval_vector_rllt_csp_hash
      , detail::is_equal_vector_rllt_csp
      > traces_map;

    typedef std::vector<traces_map> traces;

    traces traces_;
};

template<typename Base>
class offpolicy_lambda_updater : public Base
{
};

/// @file Unified view for MC and TD methods.
/// Use eligibility traces.
/// Maintain distinct traces for every agent in system.

/// @brief On-policy TD(lambda) method for state value function.
/// Algorithm described in
/// http://webdocs.cs.ualberta.ca/~sutton/book/ebook/node75.html
class method_state_onpolicy : public onpolicy_lambda_updater<method_state>
{
public:
    method_state_onpolicy(env_state* env, policy::iface* policy, const CConfigPtr& config);

private:
	/// @name method_state implementation
	/// @{
    /// @brief Update value function for specific agent with new reward
    void update_value_function_impl(
        const agent_sp& active_agent
      , int active_agent_idx
      , double new_state_value
      , double reward
      );
    /// @}
};

/// @brief On-policy TD(lambda) method for state-action value function.
/// Algorithm described in
/// http://webdocs.cs.ualberta.ca/~sutton/book/ebook/node64.html
class method_action_onpolicy : public onpolicy_lambda_updater<method_action> 
{
public:
    method_action_onpolicy(env_action* env, policy::iface* policy, const CConfigPtr& config);

protected:
    /// @brief Update value function for specific agent with new reward
    void update_value_function_impl(
        const agent_sp& active_agent
      , int active_agent_idx
      , const std::pair<double, vector_rllt_csp>& policy_selection
      , const std::pair<double, vector_rllt_csp>& greedy_selection
      , double reward
      );
};

}}
