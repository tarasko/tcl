#pragma once

#include <vector>
#include <stdexcept>
#include <memory>

#if defined(TCL_RLL_DOUBLE)
typedef double rll_type;
#define TCL_RLL_TYPE_FOR_PRINTF "%8f"
#elif defined(TCL_RLL_INT)
typedef int rll_type;
#define TCL_RLL_TYPE_FOR_PRINTF "%d"
#else
#error "Either TCL_RLL_INT or TCL_RLL_DOUBLE must be defined"
#endif

namespace tcl { namespace rll {

typedef std::vector<double> vector_dbl;
typedef std::vector<rll_type> vector_rllt;
typedef std::shared_ptr<vector_rllt> vector_rllt_sp;
typedef std::shared_ptr<const vector_rllt> vector_rllt_csp;

struct config;
class agent;
class state;
class state_with_reserved_action;
class env_base;
class env_state;
class env_action;

namespace policy {
class iface;
class greedy;
class egreedy;
}

namespace value_function {
class iface;
class lookup_table;
class neuronal_network;
}

typedef std::shared_ptr<agent> agent_sp;

}}
