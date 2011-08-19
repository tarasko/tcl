#include "mc_method.hpp"

#include <algorithm>

using std::transform;
using std::make_pair;

namespace tcl { namespace rll {

#if 0

void CMCMethod::applyHistory() {
  if (!m_history.empty() && m_history.size() != 1) {
    CUpdateList update(m_history.size() - 1);
    CHistory::reverse_iterator iHist = ++m_history.rbegin();
    CUpdateList::iterator iUpd = update.begin();
    double ret = m_history.back().second;
    for (; iHist != m_history.rend(); ++iHist, ++iUpd) {
      double val = m_ptrFunc->GetValue(iHist->first);
      double delta = m_ptrConfig->m_alpha * (ret - val);
      iUpd->first = iHist->first;
      iUpd->second = val + delta;
      ret = ret * m_ptrConfig->m_gamma + iHist->second;
    }
    m_ptrFunc->Update(update);
  }
  m_history.clear();
}

void CMCStateMethod::Reset(CStatePtr i_ptrState) {
  applyHistory();
  m_ptrFunc->Translate(i_ptrState, CActionPtr(), m_activeAgent, m_state);
  m_history.push_back(make_pair(m_state, 0.0));
}

void CMCActionMethod::Reset(CStatePtr i_ptrState, CActionPtr i_ptrAction) {
  applyHistory();
  m_ptrFunc->Translate(i_ptrState, i_ptrAction, m_activeAgent, m_state);
  m_history.push_back(make_pair(m_state, 0.0));
}

CStatePtr CMCMethodState::SelectNext(const CPossibleStates& i_states) {
  // Ok get value for every possible state
  typedef std::multimap<double, CStatePtr> CValueMap;
  CValueMap variants;
  for (CPossibleStates::const_iterator i=i_states.begin(); i!=i_states.end(); ++i) {
    double stateValue = m_ptrFunc->GetValue(*i, CActionPtr(), m_activeAgent);
    variants.insert(make_pair(stateValue, *i));
  }
  // Select action and therefore next state according policy 
  CValueMap::const_iterator variant = m_policy.Select(variants);
  m_ptrNextNotObservedState = variant->second;
  return variant->second;
}

void CMCMethodState::Reward(double i_reward) {
  CVectorDbl newState;
  m_ptrFunc->Translate(m_ptrNextNotObservedState, CActionPtr(), m_activeAgent, newState);
  m_history.push_back(make_pair(newState, i_reward));
}

#endif

}}
