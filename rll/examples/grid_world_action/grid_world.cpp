#include "grid_world.hpp"

#include <tcl/rll/lookup_table.hpp>

#include <map>
#include <functional>
#include <iterator>
#include <fstream>
#include <iostream>

using namespace tcl::rll;
using namespace std;

CGridWorld::CGridWorld(void) 
{
    // Create state
    m_state = make_shared<CState>();
    m_state->RegisterVariable("ROW");
    m_state->RegisterVariable("COLUMN");

    // Create value function and agent
    CValueFunctionPtr ptrFunc = make_shared<CLookupTable>();
    agents().push_back(make_shared<CAgent>(ptrFunc));

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

void CGridWorld::initEpisode() 
{
    m_state->SetValue("ROW", 3);
    m_state->SetValue("COLUMN", 0);
}

size_t CGridWorld::activeAgent() const
{
    return 0;
}

CStatePtr CGridWorld::currentState() const
{
    return m_state;
}

std::vector<tcl::rll::CActionPtr> CGridWorld::getPossibleActions() const
{
    std::vector<tcl::rll::CActionPtr> res;
    std::copy_if(m_actions.begin(), m_actions.end(), back_inserter(res), 
        bind(&CGridWorldAction::IsPossibleInState, placeholders::_1, m_state, cref(m_wind)));

    return res;
}

bool CGridWorld::doActionAssignRewards(const tcl::rll::CActionPtr& action)
{
    static_pointer_cast<CGridWorldAction>(action)->ChangeState(m_state, m_wind);

    bool finished = m_state->GetValue("ROW") == 3 && m_state->GetValue("COLUMN") == 7;

    if (finished)
    {
        agents()[0]->addReward(1.0);
        std::cout 
            << "Episode number:" << episode() << "\t" 
            << "Episode takes: " << step() << std::endl;
    }
    else
        agents()[0]->addReward(-1.0);

    return !finished;
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
