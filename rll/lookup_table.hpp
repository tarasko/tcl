#pragma once

#include "types.hpp"
#include "value_function.hpp"
#include "state.hpp"
#include "action.hpp"
#include "config.hpp"

#include <map>

namespace tcl { namespace rll {

/// @brief Implement value function based on lookup table.
/// Works only for state (and action) variables which has type int.
/// @todo Use hash table
class CLookupTable : public CValueFunction {
public:
  CLookupTable(double i_init = 0.0) : m_init(i_init) {}

  /** @brief Return value for internal representation of state */
  virtual double        GetValue(CVectorAnyPtr i_ptrState);
  /** @brief Correct value function according update map */
  virtual void          Update(const CUpdateList& i_list);

protected:
  typedef std::map<CVectorAnyPtr, double, PVectorAnyPtrLessStrict<int> > CValueMap;

  /** @brief Map from data vector to value */
  CValueMap             m_values;
  /** @brief Initial value for new states */
  double                m_init;
};

}}
