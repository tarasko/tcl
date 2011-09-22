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

#define DEFINE_EXCEPTION(_className)                                          \
struct _className : std::runtime_error                                        \
{                                                                             \
    _className() : runtime_error( "CRLException" ) {}                         \
    _className(const char* i_desc) : runtime_error(i_desc) {}                 \
};

namespace tcl { namespace rll {

typedef std::vector<double> vector_dbl;
typedef std::vector<rll_type> vector_rllt;
typedef std::shared_ptr<vector_rllt> vector_rllt_sp;
typedef std::shared_ptr<const vector_rllt> vector_rllt_csp;

DEFINE_EXCEPTION(CRLException)

struct CConfig;
class agent;
class state;
class state_with_reserved_action;
class env_base;
class env_state;
class env_action;
class method_state;
class method_action;
class value_function;

typedef std::shared_ptr<CConfig> CConfigPtr;
typedef std::shared_ptr<agent> agent_sp;
typedef std::shared_ptr<value_function> value_function_sp;

}}
