#ifndef __GRIDWORLD_H__
#define __GRIDWORLD_H__

#include <tcl/rll/environment.hpp>

class CGridWorld :
    public tcl::rll::CEnvAction {
public:
  CGridWorld(void);

  /** @brief Describe different moves in grid world */
  class CGridWorldAction : public tcl::rll::CAction {
  public:
    CGridWorldAction() : m_x(0), m_y(0), m_id(0) {}
    CGridWorldAction(int i_x, int i_y, int i_id) : m_x(i_x), m_y(i_y), m_id(i_id) {}

    virtual int GetData() const {
      return m_id;
    }

    void ChangeState(tcl::rll::CStatePtr i_ptrState, std::map<int, int>& i_wind) {
      int curRow = i_ptrState->GetValue("ROW");
      int curCol = i_ptrState->GetValue("COLUMN");
      i_ptrState->SetValue("COLUMN", applyColBounds(curCol + m_x));
      i_ptrState->SetValue("ROW", applyRowBounds(curRow + m_y + i_wind[curCol]));
    }

  protected:
    int applyRowBounds(int i_row) {
      i_row = i_row > 6 ? 6 : i_row;
      i_row = i_row < 0 ? 0 : i_row;
      return i_row;
    }

    int applyColBounds(int i_col) {
      i_col = i_col > 9 ? 9 : i_col;
      i_col = i_col < 0 ? 0 : i_col;
      return i_col;
    }

    int m_x;
    int m_y;
    int m_id;
  };

  virtual void          initEpisode();
  /** @brief Return true if we reached terminal state */
  virtual bool          isEpisodeFinished();
  /** @brief Get possible next states from current state */
  virtual void          fillPossibilities(CPossibleActions& o_actions);
  /** @brief Return next state by previous state and performed action */
  virtual tcl::rll::CStatePtr getNextState(tcl::rll::CStatePtr i_ptrState, tcl::rll::CActionPtr i_ptrAction);
  /** @brief Return reward after we get to new state */
  virtual void          observeRewards(tcl::rll::CVectorDbl& o_rewards);

  void                  PrintValueFunc();

protected:
  bool                  isTerminalState(tcl::rll::CStatePtr i_ptrState);
  int                   applyRowBounds(int i_row);
  int                   applyColBounds(int i_col);
  /** @brief Wind map */
  std::map<int, int>    m_wind;
  /** @brief Actions array */
  std::vector<std::shared_ptr<CGridWorldAction> >
                        m_actions;
};

#endif //__GRIDWORLD_H__
