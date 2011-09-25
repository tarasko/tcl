#pragma once

#include "method_base.hpp"

namespace tcl { namespace rll { namespace detail {

/// @brief Base class for methods which operate only on states
class state_method_base : public method_base 
{
public:
    typedef state state_type;

    state_method_base(env_state* env, policy::iface* policy, const config& config);

protected:
    /// @brief Process episode as states method.
    virtual void run_episode_impl();

private:
    typedef policy::iface::variants value_state_map;

    /// Used internally by run_episode_impl
    /// Makeing this as member helps to avoid allocations for next 
    /// possible state on each step.
    value_state_map variants_; 
};

}}}
