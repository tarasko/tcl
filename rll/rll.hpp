#pragma once

#include "agent.hpp"
#include "config.hpp"
#include "environment.hpp"
#include "lambda_method.hpp"
#include "state.hpp"
#include "policy_greedy.hpp"
#include "policy_egreedy.hpp"
#if defined(TCL_RLL_INT)
#include "vf_lookup_table.hpp"
#endif
#include "vf_neuronal_network.hpp"
