#include "lambda_method.hpp"
#include "agent.hpp"
#include "environment.hpp"

#include <cassert>
#include <functional>

namespace tcl { namespace rll { 

template<typename Base>
void onpolicy_lambda_updater<Base>::update_value_function_helper(
    const agent_sp& active_agent
  , size_t active_agent_idx
  , const vector_rllt_csp& old_state
  , double new_state_value
  , double reward
  )
{
    traces_map& active_agent_traces = traces_[active_agent_idx];

    double v = active_agent->get_value(old_state);
    double new_v = new_state_value;

    agent::update_list updates;

    double temp = reward - v + config_->m_gamma * new_v;

    // Update eligibility trace for state
    // Accumulating traces: e(s) <- e(s) + 1
    // Replacing traces: e(s) <- 1 
    // Search trace
    traces_map::iterator i = active_agent_traces.find(old_state);

    if (active_agent_traces.end() == i) 
    {
        // No trace found. Add it.
        active_agent_traces.insert(make_pair(old_state, 1.0));
    } 
    else 
    {
        // Trace found. Update it.
        // Accumulating or replacing traces?
        if (config_->m_accumulating)
            ++i->second;
        else
            i->second = 1;
    }

    // Fill update map
    // Run over all past agent states	
    for (traces_map::iterator i = active_agent_traces.begin(); i != active_agent_traces.end();) 
    {
        double change = config_->m_alpha * temp * i->second;
        updates.push_back(make_pair(i->first, active_agent->get_value(i->first) + change));
        // Reduce trace
        // TODO: config_->m_lambda * config_->m_gamma we can do it on startup
        i->second *= (config_->m_lambda * config_->m_gamma);

        // Erase traces that become less then config_->m_etEpsilon
        if (i->second < config_->m_etEpsilon)
        {
            traces_map::iterator toDelete = i++;
            active_agent_traces.erase(toDelete);
        }
        else 
            ++i;
    }

    // Update value function
    active_agent->update(updates);
}

template<typename Base>
void onpolicy_lambda_updater<Base>::run_episode_impl()
{
    std::for_each(traces_.begin(), traces_.end(), std::mem_fn(&traces_map::clear));
    Base::run_episode_impl();
}

method_state_onpolicy::method_state_onpolicy(env_state* env, const CConfigPtr& config) 
    : onpolicy_lambda_updater<method_state>(env, config)
{
}

void method_state_onpolicy::update_value_function_impl(
    const agent_sp& active_agent
  , int active_agent_idx
  , double new_state_value
  , double reward
  )
{
    update_value_function_helper(
        active_agent
      , active_agent_idx
      , active_agent->prev_state()
      , new_state_value
      , reward
      );
}

method_action_onpolicy::method_action_onpolicy(env_action* env, const CConfigPtr& config)
    : onpolicy_lambda_updater<method_action>(env, config)
{
}

void method_action_onpolicy::update_value_function_impl(
    const agent_sp& active_agent
  , int active_agent_idx
  , const std::pair<double, vector_rllt_csp>& policy_selection
  , const std::pair<double, vector_rllt_csp>&
  , double reward
  )
{
    update_value_function_helper(
        active_agent
      , active_agent_idx
      , active_agent->prev_state()
      , policy_selection.first
      , reward
      );
}

}}