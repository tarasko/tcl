#pragma once

#include "../agent.hpp"
#include "../policy/iface.hpp"

#include "method_base.hpp"
#include "utils.hpp"

#include <unordered_map>
#include <vector>
#include <functional>
#include <utility>

namespace tcl { namespace rll { namespace detail {

template<typename Base>
class lambda_method : public Base
{
public:
    template<typename EnvType> 
    lambda_method(EnvType* env, const config& config);

private:
    typedef std::unordered_map<
        vector_rllt_csp
      , double
      , detail::eval_vector_rllt_csp_hash
      , detail::is_equal_vector_rllt_csp
      > traces_map;

    typedef std::vector<traces_map> traces;

    /// @brief Reset agent traces, call underlying run_episode_impl
    virtual void run_episode_impl();

    virtual void update_value_function_impl(
        const agent_sp& active_agent
      , int active_agent_idx
      , double policy_selection_value
      , double greedy_selection_value
      , double reward
      );

    void prepare_update_reduce_traces(
        const agent_sp& active_agent
      , double temp
      , traces_map& active_agent_traces
      , agent::update_list& updates
      );

    void prepare_update_delete_traces(
        const agent_sp& active_agent
      , double temp
      , traces_map& active_agent_traces
      , agent::update_list& updates
      );

    traces traces_;  //!< Eligibility traces for each agent
};

template<typename Base>
template<typename EnvType> 
inline lambda_method<Base>::lambda_method(
    EnvType* env
  , const config& config
  )
  : Base(env, config)
  , traces_(env->agents().size())
{
}

template<typename Base>
inline void lambda_method<Base>::prepare_update_reduce_traces(
    const agent_sp& active_agent
  , double temp
  , traces_map& active_agent_traces
  , agent::update_list& updates
  )
{
    for (traces_map::iterator i = active_agent_traces.begin(); i != active_agent_traces.end();)
    {
        double change = Base::config_.alpha_ * temp * i->second;
        updates.push_back(
            make_pair(i->first, active_agent->vf().get_value(i->first) + change)
          );
        // Reduce trace
        // TODO: config_.lambda_ * config_.gamma_ we can do it on startup
        i->second *= (Base::config_.lambda_ * Base::config_.gamma_);

        // Erase traces that become less then config_.traces_epsilon_
        if (i->second < Base::config_.traces_epsilon_)
        {
            traces_map::iterator to_delete = i++;
            active_agent_traces.erase(to_delete);
        }
        else
            ++i;
    }
}

template<typename Base>
inline void lambda_method<Base>::prepare_update_delete_traces(
    const agent_sp& active_agent
  , double temp
  , traces_map& active_agent_traces
  , agent::update_list& updates
  )
{
    std::for_each(active_agent_traces.begin(), active_agent_traces.end(),
        [&](traces_map::const_reference r) -> void
        {
            double change = config_.alpha_ * temp * r.second;
            updates.push_back(
                make_pair(r.first, active_agent->vf().get_value(r.first) + change)
              );
        }
    );

    active_agent_traces.clear();
}

template<typename Base>
void lambda_method<Base>::run_episode_impl()
{
    std::for_each(traces_.begin(), traces_.end(), std::mem_fn(&traces_map::clear));
    Base::run_episode_impl();
}

template<typename Base>
void lambda_method<Base>::update_value_function_impl(
    const agent_sp& active_agent
  , int active_agent_idx
  , double policy_selection_value
  , double greedy_selection_value
  , double reward
  )
{
    traces_map& active_agent_traces = traces_[active_agent_idx];
    auto old_state = active_agent->prev_state();

    double v = active_agent->vf().get_value(old_state);
    double new_v = active_agent->onpolicy() ? policy_selection_value : greedy_selection_value;

    double temp = reward - v + Base::config_.gamma_ * new_v;

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
        if (Base::config_.accumulating_)
            ++i->second;
        else
            i->second = 1;
    }

    // Fill update map
    // Run over all past agent states
    agent::update_list updates;

    if (active_agent->onpolicy() || (policy_selection_value == greedy_selection_value))
        prepare_update_reduce_traces(active_agent, temp, active_agent_traces, updates);
    else
        prepare_update_delete_traces(active_agent, temp, active_agent_traces, updates);

    // Update value function
    active_agent->vf().update(updates);
}

}}}
