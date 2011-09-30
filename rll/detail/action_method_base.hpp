#pragma once

#include "method_base.hpp"
#include "../policy/iface.hpp"
#include "../environment.hpp"

namespace tcl { namespace rll { namespace detail {

/// @brief Base class for methods which operate on state-action pairs
class action_method_base : public method_base 
{
public:
    typedef state_with_reserved_action state_type;

    action_method_base(env_action* env, const config& config);

protected:
    /// @brief Process episode as actions-states method.
    virtual void run_episode_impl();

private:
    typedef policy::iface::variants value_action_map;

    /// Used internally by run_episode_impl
    /// Makeing this as member helps to avoid allocations for next 
    /// possible state on each step.
    value_action_map variants_; 
};

inline action_method_base::action_method_base(env_action* env, const config& config) 
    : method_base(env, config)
{
}

}}}
