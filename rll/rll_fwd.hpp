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

typedef std::vector<double> CVectorDbl;
typedef std::vector<int> CVectorInt;
typedef std::vector<rll_type> CVectorRllt;
typedef std::shared_ptr<CVectorRllt> CVectorRlltPtr;

DEFINE_EXCEPTION(CRLException)

struct CConfig;
class CState;
class CAction;
class CEnvBase;
class CEnvState;
class CEnvAction;
class CAgent;
class CStateMethod;
class CActionMethod;
class CPolicy;
class CValueFunction;

typedef std::shared_ptr<CConfig> CConfigPtr;
typedef std::shared_ptr<CState> CStatePtr;
typedef std::shared_ptr<CAction> CActionPtr;
typedef std::shared_ptr<CAgent> CAgentPtr;
typedef std::shared_ptr<CStateMethod> CStateMethodPtr;
typedef std::shared_ptr<CActionMethod> CActionMethodPtr;
typedef std::shared_ptr<CPolicy> CPolicyPtr;
typedef std::shared_ptr<CValueFunction> CValueFunctionPtr;

}}
