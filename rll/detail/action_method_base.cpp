#include "action_method_base.hpp"
#include "../agent.hpp"
#include "../environment.hpp"

#include <functional>

namespace tcl { namespace rll { namespace detail {

action_method_base::action_method_base(env_action* env, policy::iface* policy, const config& config) 
    : method_base(env, policy, config)
{
}

void action_method_base::run_episode_impl() 
{
    env_action* env = static_cast<env_action*>(env_);

    env->init_episode();
    bool cont = true;
    
    // 1. Get possible actions for active agent.
    // 2. Evaluate all state-action representations, get estimated values for them.
    // 3. Select state-action according policy.
    // 4. Update value function. We can do this step cause we know Q(t-1) and Q(t) and last reward.
    // In case of first move we don`t do update.
    // Probably here must be offline and online updates.
    // 5. Make last selected action, agents can recieve rewards on this step. 
    // All this rewards are remembered until we will know next state-action pair.
    // On this step environment also can tell that we reached terminal state.
    // If we are in terminal state go to step 6. Else go to step 1.
    // 6. Iterate over all agents, pretend that next state-action pair will have value function 0.0
    // Update according to last reward.

    for (step_ = 0; cont; ++step_)
    {
        // Get active agent and it index
        int active_agent_idx = env_->active_agent();
        agent_sp active_agent = env_->agents()[active_agent_idx];

        // 1. Get possible actions for active agent.
        auto possible_actions = env->get_possible_actions();
        if (possible_actions.empty())
            throw std::logic_error("at least must be one possible next action");

        state_type current_state = env->current_state();

        // 2. Evaluate all state-action representations, get estimated values for them
        variants_.resize(possible_actions.size());
        std::transform(
            possible_actions.begin()
          , possible_actions.end()
          , variants_.begin()
          , [&](rll_type a) -> std::pair<double, vector_rllt_sp>
            {
                auto rep = current_state.clone().get_internal_rep(a);
                double value = active_agent->get_value(rep);
                return std::make_pair(value, rep);
            }
          );

        // Sort it
        std::sort(
            variants_.begin()
          , variants_.end()
          , [](value_action_map::const_reference r1, value_action_map::const_reference r2) -> bool
            {
                return r1.first < r2.first;
            }
          );

        // 3. Select state-action according policy.
        value_action_map::const_reference policy_selection = policy_->select(variants_);
        value_action_map::const_reference greedy_selection = variants_.back();

        // 4. Update value function. We can do this step cause we know Q(t-1) and Q(t) and last reward.
        // In case of first move we don`t do update.
        // Probably here must be offline and online updates.
        if (active_agent->prev_state()) {
            update_value_function_impl(
                active_agent
              , active_agent_idx
              , policy_selection.first
              , greedy_selection.first
              , active_agent->release_reward()
              );
        }

        active_agent->set_prev_state(policy_selection.second);

        // 5. Make last selected action, agents can recieve rewards on this step. 
        // All this rewards are remembered until we will know next state-action pair.
        // On this step environment also can tell that we reached terminal state.
        // If we are in terminal state go to step 5. Else go to step 1.
        cont = env->do_action_assign_rewards(policy_selection.second->back());
    }

    // Get terminal rewards for all agents
    vector_dbl terminal_rewards(env_->agents().size());
    std::transform(
        env_->agents().begin()
      , env_->agents().end()
      , terminal_rewards.begin()
      , std::mem_fn(&agent::release_reward)
      );

    // 6. Iterate over all agents, pretend that next state-action pair will have value function 0.0
    // Update according to last reward.
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
