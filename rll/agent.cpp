#include "agent.hpp"

#include <cassert>
#include <algorithm>

using std::make_pair;

namespace tcl { namespace rll {

CAgent::CAgent(const CValueFunctionPtr& i_ptrFunc) 
    : m_reward(0.0)
    , m_ptrFunc(i_ptrFunc) 
{
    assert(m_ptrFunc != NULL);
}

void CAgent::UpdateTrace(
    const CVectorRlltPtr& i_ptrState
  , bool i_accum
  , double i_etEpsilon
  ) 
{
    // Search trace
    CTracesMap::iterator i = std::find_if(
        m_traces.begin()
      , m_traces.end()
      , [&i_ptrState](CTracesMap::const_reference r)
        {
            return std::equal(
                i_ptrState->begin()
              , i_ptrState->end()
              , r.first->begin()
              );
        }
      );

    if (m_traces.end() == i) {
        // No trace found. Add it.
        m_traces.push_back(make_pair(i_ptrState, 1.0));
    } else {
        // Trace found. Update it.
        // Accumulating or replacing traces?
        if (i_accum) {
            ++i->second;
        } else {
            i->second = 1;
        }
    }

    // Ok run over traces and remove elements with trace less then 
    // i_etEpsilon
    m_traces.remove_if([i_etEpsilon](CTracesMap::const_reference r)
        {
            return r.second < i_etEpsilon;
        }
      );
}

void CAgent::Clean() 
{
    // Reset accumulated reward
    m_reward = 0.0;
    // Set m_ptrPrevState to NULL
    m_ptrPrevState.reset();
    // Clean traces
    m_traces.clear();
}

}}
