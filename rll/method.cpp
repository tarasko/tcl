#include "method.hpp"
#include "environment.hpp"
#include "agent.hpp"
#include "state.hpp"

#include <functional>
#include <algorithm>

namespace tcl { namespace rll {

method_base::method_base(env_base* env, detail::policy* policy, const CConfigPtr& config) 
    : config_(config)
    , policy_(policy) 
    , env_(env)
    , episode_(0)
    , step_(0)
{
    assert(env && "Environment cannot be null");
    assert(pol && "Policy cannot be null");
    assert(config && "Config cannot be null");
}

method_base::~method_base()
{
}

void method_base::run(unsigned int episodes) 
{
    env_->method_ = this;

    for (episode_ = 0; episode_ < episodes; ++episode_) try
    {
        step_ = 0;

        // Reset previous state for all agents
        std::for_each(
            env_->agents().begin()
          , env_->agents().end()
          , [](agent_sp& agent) -> void
            { 
                agent->set_prev_state(vector_rllt_csp());
            }
          );

        run_episode_impl();    
    }
    catch(...)
    {
        env_->method_ = 0;
        throw;
    }

    env_->method_ = 0;
}

unsigned int method_base::episode() const
{
    return episode_;
}

unsigned int method_base::step() const
{
    return step_;
}

method_state::method_state(env_state* env, detail::policy* policy, const CConfigPtr& config) 
    : method_base(env, policy, config)
{
}

void method_state::run_episode_impl() 
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
            throw CRLException("at least must be one possible next state");

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
          , terminal_rewards[agentIdx]
          );
    }
}

method_action::method_action(env_action* env, detail::policy* policy, const CConfigPtr& config) 
    : method_base(env, policy, config)
{
}

void method_action::run_episode_impl() 
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
            throw CRLException("At least must be one possible next action");

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
              , policy_selection
              , greedy_selection
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
    auto terminal_action = std::make_pair(0.0, vector_rllt_csp());
    for(size_t agentIdx = 0; agentIdx < terminal_rewards.size(); ++agentIdx) 
    {
        update_value_function_impl(
            env->agents()[agentIdx]
          , agentIdx
          , terminal_action
          , terminal_action
          , terminal_rewards[agentIdx]
          );
    }
}

}}
