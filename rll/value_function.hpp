#pragma once

#include "types.hpp"
#include "state.hpp"
#include "action.hpp"

#include <vector>
#include <memory>
#include <utility>

namespace tcl { namespace rll {

/** @brief Unified view for value function.
    Every state (or state-action pair) has two views: external and 
    internal. External consist of two pointer CStatePtr and CActionPtr.
    If we working with state value function CActionPtr is null. 
    The internal is a vector of boost::any and can strongly differ from 
    external representation. Suppose we use tile coding. The external 
    representation is the exact state and the internal is set of binary 
    features which cover the state. The internal state also defines sense 
    when one state equals to another. */
class CValueFunction 
{
public:
    typedef std::vector<std::pair<CVectorRlltPtr, double> > CUpdateList;

    /** @brief Return value for internal representation of state */
    virtual double GetValue(const CVectorRlltPtr& i_ptrState) = 0;

    /** @brief Correct value function according update map */
    virtual void Update(const CUpdateList& i_list) = 0;

    /** @brief Return value for state-action pair.
    If we use state value function pass null action pointer */
    virtual double GetValue(
        const CStatePtr& i_ptrState
      , const CActionPtr& i_ptrAction
      , int i_agent
      ) 
    {
        CVectorRlltPtr ptrIntState = translate(i_ptrState, i_ptrAction, i_agent);
        return GetValue(ptrIntState);
    }
};

}}
