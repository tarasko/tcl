#include "grid_world.hpp"
#include <tcl/rll/lambda_method.hpp>
#include <tcl/rll/lookup_table.hpp>

#include <map>
#include <fstream>
#include <iostream>

using namespace tcl::rll;
using namespace std;

CGridWorld::CGridWorld(void) {
  // Create config
  CConfigPtr ptrConfig(new CConfig);
  ptrConfig->m_enableLog = false;

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
    }*/

    
    std::cout << "Episode number:" << m_episode << "\t" <<
      "Episode takes: " << m_step + 1 << std::endl;
    return true;
  };
  return false;
}

void CGridWorld::fillPossibilities(CPossibleStates& o_states) {
  // Spawn four new states which occurs after corresponding actions and choose 
  // some according to policy

  int curRow = m_ptrState->GetValue("ROW");
  int curCol = m_ptrState->GetValue("COLUMN");

  // Move left
  {
    CStatePtr ptrNewState = m_ptrState->Clone();
    ptrNewState->SetValue("COLUMN", applyColBounds(curCol - 1));
    ptrNewState->SetValue("ROW", applyRowBounds(curRow + m_wind[curCol]));
    if (!m_ptrState->IsEqual(ptrNewState)) {
      o_states.push_back(ptrNewState);
    }
  }
  
  // Move right
  {
    CStatePtr ptrNewState = m_ptrState->Clone();
    ptrNewState->SetValue("COLUMN", applyColBounds(curCol + 1));
    ptrNewState->SetValue("ROW", applyRowBounds(curRow + m_wind[curCol]));
    if (!m_ptrState->IsEqual(ptrNewState)) {
      o_states.push_back(ptrNewState);
    }
  }

  // Move top
  {
    CStatePtr ptrNewState = m_ptrState->Clone();
    ptrNewState->SetValue("ROW", applyRowBounds(curRow + 1 + m_wind[curCol]));
    if (!m_ptrState->IsEqual(ptrNewState)) {
      o_states.push_back(ptrNewState);
    }
  }

  // Move bottom
  {
    CStatePtr ptrNewState = m_ptrState->Clone();
    ptrNewState->SetValue("ROW", applyRowBounds(curRow - 1 + m_wind[curCol]));
    if (!m_ptrState->IsEqual(ptrNewState)) {
      o_states.push_back(ptrNewState);
    }
  }
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
  std::ofstream fout("out.txt");
  CStatePtr temp = m_ptrState->Clone();
  for (int row = 6; row >= 0; --row) {
    temp->SetValue("ROW", row);
    for (int col = 0; col < 10; ++col) {
      temp->SetValue("COLUMN", col);
      fout.width(9);
      fout << m_agents[0]->m_ptrFunc->GetValue(translate(temp, CActionPtr(), 0)) << " ";
    }
    fout << std::endl;
  }
}

int CGridWorld::applyRowBounds(int i_row) {
  i_row = i_row > 6 ? 6 : i_row;
  i_row = i_row < 0 ? 0 : i_row;
  return i_row;
}

int CGridWorld::applyColBounds(int i_col) {
  i_col = i_col > 9 ? 9 : i_col;
  i_col = i_col < 0 ? 0 : i_col;
  return i_col;
}
