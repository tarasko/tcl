#ifndef __RLDREAM_MCMETHOD_H__
#define __RLDREAM_MCMETHOD_H__

#include "method.hpp"

/** @file Set of Monte-Carlo methods */

namespace tcl { namespace rll {

#if 0

class CMCMethod : 
  public CMethodBase {
public:
  CMCMethod(CConfigPtr i_ptrConfig, CValueFunctionPtr i_ptrFunc) :
   CMethodBase(i_ptrConfig, i_ptrFunc) {}

  /** @brief Apply history to value function and clear history */
  void                  applyHistory();
protected:
  /** @brief Vector of pairs "internal state - reward" */
  typedef std::vector<std::pair<CVectorDbl, double> > CHistory;
  typedef CValueFunction::CUpdateList CUpdateList;
  /** @brief History of all states and recieved rewards */
  CHistory              m_history;
};

/** @brief Implement Reset method
  Base class for state methods with rewards history */
class CMCStateMethod :
  public CMCMethod,
  public CStateMethod {
public:
  CMCStateMethod(CConfigPtr i_ptrConfig, CValueFunctionPtr i_ptrFunc) :
   CMCMethod(i_ptrConfig, i_ptrFunc) {}

  /** @brief Reset to some initial state and action.
    Actually this is the way how up layer tell us that episode 
    has finished. */
  virtual void          Reset(CStatePtr i_ptrState);
};

/** @brief Implement Reset method
  Base class for state-action methods with rewards history */
class CMCActionMethod :
  public CMCMethod,
  public CActionMethod {
public:
  CMCActionMethod(CConfigPtr i_ptrConfig, CValueFunctionPtr i_ptrFunc) :
   CMCMethod(i_ptrConfig, i_ptrFunc) {}

  /** @brief Reset to some initial state and action. */
  virtual void          Reset(CStatePtr i_ptrState, CActionPtr i_ptrAction);
};

class CMCMethodState :
  public CMCStateMethod {
public:
  CMCMethodState(CConfigPtr i_ptrConfig, CValueFunctionPtr i_ptrFunc) :
   CMCStateMethod(i_ptrConfig, i_ptrFunc) {}

  virtual CStatePtr     SelectNext(const CPossibleStates& i_states);
  /** @brief Recieve reward after selecting next state */
  virtual void          Reward(double i_reward);
};

class CMCMethodOnpolicy :
  public CMCActionMethod {
public:
  CMCMethodOnpolicy(CConfigPtr i_ptrConfig, CValueFunctionPtr i_ptrFunc) :
   CMCActionMethod(i_ptrConfig, i_ptrFunc) {}

  virtual CStatePtr     SelectNext(const CPossibleActions& i_states) = 0;
  /** @brief Recieve reward after selecting next state */
  virtual void          Reward(double i_reward) = 0;
};

class CMCMethodOffpolicy :
  public CMCActionMethod {
public:
  CMCMethodOffpolicy(CConfigPtr i_ptrConfig, CValueFunctionPtr i_ptrFunc) :
   CMCActionMethod(i_ptrConfig, i_ptrFunc) {}

  virtual CStatePtr     SelectNext(const CPossibleActions& i_states) = 0;
  /** @brief Recieve reward after selecting next state */
  virtual void          Reward(double i_reward) = 0;
};

#endif

}}

#endif //__RLDREAM_MCMETHOD_H__
