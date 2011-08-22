#include "grid_world.hpp"

#include <tcl/rll/lookup_table.hpp>

#include <map>
#include <fstream>
#include <iostream>

using namespace tcl::rll;
using namespace std;

CGridWorld::CGridWorld(void) {
  // Create state
  m_ptrState.reset(new CState());
  m_ptrState->RegisterVariable("ROW");
  m_ptrState->RegisterVariable("COLUMN");

  // Create value function and method
  CValueFunctionPtr ptrFunc(new CLookupTable);
  CAgentPtr ptrAgent(new CAgent(ptrFunc));
  m_agents.push_back(ptrAgent);

  // Init wind
  m_wind[0] = 0;
  m_wind[1] = 0;
  m_wind[2] = 0;
  m_wind[3] = 1;
  m_wind[4] = 1;
  m_wind[5] = 1;
  m_wind[6] = 2;
  m_wind[7] = 2;
  m_wind[8] = 1;
  m_wind[9] = 0;

  // Init actions
  m_actions.push_back(std::make_shared<CGridWorldAction>(-1, 0, 0));
  m_actions.push_back(std::make_shared<CGridWorldAction>(1, 0, 1));
  m_actions.push_back(std::make_shared<CGridWorldAction>(0, -1, 2));
  m_actions.push_back(std::make_shared<CGridWorldAction>(0, 1, 3));
}

void CGridWorld::initEpisode() {
  m_ptrState->SetValue("ROW", 3);
  m_ptrState->SetValue("COLUMN", 0);
}

bool CGridWorld::isEpisodeFinished() {
  static int sum = 0;
  if (isTerminalState(m_ptrState)) {
/*    sum += m_step + 1;
    if ((m_episode + 1) % 10 == 0) {
      double avg = sum / 10.0;
      std::cout << m_episode + 1 << "\t" << avg << std::endl;
      sum = 0;
    }
*/
    
    std::cout << "Episode number:" << m_episode + 1 << "\t" <<
      "Episode takes: " << m_step + 1 << std::endl;
    return true;
  };
  return false;
}

void CGridWorld::fillPossibilities(CPossibleActions& o_actions) {
  o_actions.assign(m_actions.begin(), m_actions.end());
}

CStatePtr CGridWorld::getNextState(CStatePtr i_ptrState, CActionPtr i_ptrAction) {
  CStatePtr ptrNewState = i_ptrState->Clone();
  static_pointer_cast<CGridWorldAction>(i_ptrAction)->ChangeState(ptrNewState, m_wind);
  return ptrNewState;
}

void CGridWorld::observeRewards(CVectorDbl& o_rewards) {
  // Trace state
//  std::cout << "(" << any_cast<int>(i->second->GetValue("ROW")) << "," <<
//    any_cast<int>(i->second->GetValue("COLUMN")) << ")\n";
  o_rewards[0] = isTerminalState(m_ptrState) ? 1.0 : -1.0;
}

bool CGridWorld::isTerminalState(CStatePtr i_ptrState) {
  return i_ptrState->GetValue("ROW") == 3 && 
         i_ptrState->GetValue("COLUMN") == 7;
}

void CGridWorld::PrintValueFunc() {
/*  std::ofstream fout("out.txt");
  CStatePtr temp = m_ptrState->Clone();
  for (int row = 6; row >= 0; --row) {
    temp->SetValue("ROW", row);
    for (int col = 0; col < 10; ++col) {
      temp->SetValue("COLUMN", col);
      fout.width(9);
      fout << m_ptrFunc->GetValue(temp, CActionPtr(), 0) << " ";
    }
    fout << std::endl;
  }
*/
}

int CGridWorld::applyRowBounds(int i_row) 
{
    i_row = i_row > 6 ? 6 : i_row;
    i_row = i_row < 0 ? 0 : i_row;
    return i_row;
}

int CGridWorld::applyColBounds(int i_col) 
{
    i_col = i_col > 9 ? 9 : i_col;
    i_col = i_col < 0 ? 0 : i_col;
    return i_col;
}
