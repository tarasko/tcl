#pragma once

#include <vector>
#include <functional>
#include <memory>
#include <exception>

#define DEFINE_EXCEPTION(_className)                                          \
struct _className : std::exception                                            \
{                                                                             \
    _className() {}                                                           \
    _className(const char* i_desc) : exception(i_desc) {}                     \
};

namespace tcl { namespace rll {

typedef std::vector<double> CVectorDbl;
typedef std::shared_ptr<CVectorDbl> CVectorDblPtr;

DEFINE_EXCEPTION(CRLException)

class CState;
class CAction;

typedef std::shared_ptr<CState> CStatePtr;
typedef std::shared_ptr<CAction> CActionPtr;

/// @brief Translate from external state to internal.
///
/// If we use state value function pass null action pointer.
/// Active agent index append to vector as last element
CVectorDblPtr translate(
    const CStatePtr& i_ptrState
  , const CActionPtr& i_ptrAction
  , int i_agent
  );

}}
