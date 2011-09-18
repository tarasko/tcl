#include "agent.hpp"
#include "value_function.hpp"

#include <cassert>
#include <algorithm>

using std::make_pair;

namespace tcl { namespace rll {

agent::agent(const value_function_sp& func) 
    : func_(func)
    , reward_(0.0)
{
    assert(func_);
}

double agent::get_value(const vector_rllt_csp& st)
{
    return func_->get_value(st);
}

void agent::update(const update_list& lst)
{
	func_->update(lst);
}

void agent::add_reward(double reward)
{
    reward_ += reward;
}

double agent::release_reward()
{
    double tmp = reward_;
    reward_ = 0.0;
    return tmp;
}

vector_rllt_csp agent::prev_state() const
{
    return prev_state_;
}

void agent::set_prev_state(const vector_rllt_csp& prev_state)
{
    prev_state_ = prev_state;
}

}}
