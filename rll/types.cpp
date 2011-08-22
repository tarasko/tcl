#include "types.hpp"
#include "state.hpp"
#include "action.hpp"

namespace tcl { namespace rll {

CVectorRlltPtr translate(
    const CStatePtr& i_ptrState
  , const CActionPtr& i_ptrAction
  , int i_agent) 
{
    CVectorRlltPtr ptrRet = i_ptrState->GetData();

    if (i_ptrAction) {
        ptrRet->push_back(i_ptrAction->GetData());
    }
    ptrRet->push_back(i_agent);
    return ptrRet;    
}

}}
