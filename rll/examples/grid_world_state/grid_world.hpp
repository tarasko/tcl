#ifndef __GRIDWORLD_H__
#define __GRIDWORLD_H__

#include <tcl/rll/rll.hpp>

class CGridWorld :
    public tcl::rll::CEnvState {
public:
  CGridWorld(void);

  virtual void          initEpisode();
  /** @brief Return true if we reached terminal state */
  virtual bool          isEpisodeFinished();
  /** @brief Get possible next states from current state */
  virtual void          fillPossibilities(CPossibleStates& o_states);
  /** @brief Return reward after we get to new state */
  virtual void          observeRewards(tcl::rll::CVectorDbl& o_rewards);

  void                  PrintValueFunc();

protected:
  bool                  isTerminalState(tcl::rll::CStatePtr i_ptrState);

  int                   applyRowBounds(int i_row);
  int                   applyColBounds(int i_col);

  /** @brief Wind map */
  std::map<int, int>    m_wind;
};

#endif //__GRIDWORLD_H__
