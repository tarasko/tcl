#pragma once

#include "detail/lambda_method.hpp"
#include "detail/action_method_base.hpp"
#include "detail/state_method_base.hpp"

namespace tcl { namespace rll {

typedef detail::lambda_method<detail::state_method_base> state_method;
typedef detail::lambda_method<detail::action_method_base> action_method;

}}
