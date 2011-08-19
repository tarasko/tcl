#ifndef __RLDREAM_LINEAPPROXIMATION_H__
#define __RLDREAM_LINEAPPROXIMATION_H__

#include "value_function.hpp"

namespace tcl { namespace rll {

class CLineApproximation :
  public CValueFunction {
public:
  CLineApproximation(void);
  ~CLineApproximation(void);
};

}}

#endif //__RLDREAM_LINEAPPROXIMATION_H__
