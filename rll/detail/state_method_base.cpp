#include "state_method_base.hpp"
#include "../agent.hpp"
#include "../environment.hpp"

#include <functional>

namespace tcl { namespace rll { namespace detail {

state_method_base::state_method_base(env_state* env, policy::iface* policy, const config& config) 
    : method_base(env, policy, config)
{
}

void state_method_base::run_episode_impl() 
{
    env_state* env = static_cast<env_state*>(env_);

    env->init_episode();
    
    // Repeat for each step in episode
    // Break when set_next_state_assign_rewards return false
    for(step_ = 0;; ++step_)
    {
        // Get active agent on this step
        int      active_agent_idx = env->active_agent();
        agent_sp active_agent     = env->agents()[active_agent_idx];

        // When agent do it`s first step he don`t know previous state.
        // Set current state as previous (and initial) for agent.
        if (!active_agent->prev_state()) 
            active_agent->set_prev_state(env->current_state().clone().get_internal_rep());

        // Get possible next states for current active agent
        auto next_states = env->get_possible_next_states();
        if (next_states.empty())
            throw std::logic_error("at least must be one possible next state");

        // Get estimate value for each possible new state
        variants_.resize(next_states.size());
        std::transform(
            next_states.begin()
          , next_states.end()
          , variants_.begin()
          , [&](state_type& state) -> std::pair<double, vector_rllt_sp>
            {
                vector_rllt_sp rep = state.get_internal_rep();
                double stateValue = active_agent->get_value(rep);
                return std::make_pair(stateValue, rep);
            }
          );

        // Sort this values because we must pass sorted vector to policy
        std::sort(
            variants_.begin()
          , variants_.end()
          , [](value_state_map::const_reference r1, value_state_map::const_reference r2) -> bool
            {
                return r1.first < r2.first;
            }
          );

        // Select next state according policy 
        value_state_map::const_reference policy_selection = policy_->select(variants_);
        value_state_map::const_reference greedy_selection = variants_.back();

        // Set next state get reward for agent
        // We need to clone state
        if (env->set_next_state_assign_rewards(state(policy_selection.second)))
        {
            // Update value function
            update_value_function_impl(
                active_agent
              , active_agent_idx
              , policy_selection.first
              , greedy_selection.first
              , active_agent->release_reward()
              );

            // Remember new state as previous for active agent
            active_agent->set_prev_state(policy_selection.second);
        }
        else
            break; // finish loop after we got to terminal state
    }

    // Get terminal rewards for all agents
    vector_dbl terminal_rewards(env_->agents().size());
    std::transform(
        env_->agents().begin()
      , env_->agents().end()
      , terminal_rewards.begin()
      , std::mem_fn(&agent::release_reward)
      );

    // Update value function for last state for each agent according to terminal rewards
    for(size_t agentIdx = 0; agentIdx < terminal_rewards.size(); ++agentIdx) 
    {
        update_value_function_impl(
            env->agents()[agentIdx]
          , agentIdx
          , 0.0
          , 0.0
          , terminal_rewards[agentIdx]
          );
    }
}

}}}
