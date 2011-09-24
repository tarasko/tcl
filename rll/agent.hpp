#pragma once

#include "rll_fwd.hpp"
#include "value_function/iface.hpp"

#include <cassert>

namespace tcl { namespace rll {

/// @brief Agent that participate in reinforcement learning. 
/// Generally you are supposed only to create agents on started and pass them
/// appropriate value function.
class agent 
{
public:
	typedef value_function::iface::update_list update_list;

public:
    agent(value_function::iface* func);

    void add_reward(double reward);
    double release_reward();

    // All this methods only for internal purposes.
    double get_value(const vector_rllt_csp& state);
    void update(const update_list& lst);

    vector_rllt_csp prev_state() const;
    void set_prev_state(const vector_rllt_csp& prev_state);

private:
    value_function::iface* func_;  //!< Value function for agent.
    vector_rllt_csp prev_state_;   //!< Agent previous state when was active.
    double reward_;                //!< Accumulated agent rewards since last time he was active.
};

inline agent::agent(value_function::iface* func) 
    : func_(func)
    , reward_(0.0)
{
    assert(func_);
}

inline double agent::get_value(const vector_rllt_csp& st)
{
    return func_->get_value(st);
}

inline void agent::update(const update_list& lst)
{
	func_->update(lst);
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

inline vector_rllt_csp agent::prev_state() const
{
    return prev_state_;
}

inline void agent::set_prev_state(const vector_rllt_csp& prev_state)
{
    prev_state_ = prev_state;
}

}}
