#include "grid_world.hpp"
#include <tcl/rll/lambda_method.hpp>
#include <tcl/rll/lookup_table.hpp>

#include <map>
#include <fstream>
#include <iostream>

using namespace tcl::rll;
using namespace std;

CGridWorld::CGridWorld(void) 
{
    // Create config
    CConfigPtr ptrConfig(new CConfig);
    ptrConfig->m_enableLog = false;

    // Create state
    CStatePtr ptrState(new CState);
    ptrState->RegisterVariable("ROW");
    ptrState->RegisterVariable("COLUMN");
    setCurrentState(ptrState);

    // Create value function and method
    CValueFunctionPtr ptrFunc(new CLookupTable);
    CAgentPtr ptrAgent(new CAgent(ptrFunc));
    agents().push_back(ptrAgent);

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

void CGridWorld::initEpisodeImpl() 
{
    CStatePtr ptrState = currentState();
    ptrState->SetValue("ROW", 3);
    ptrState->SetValue("COLUMN", 0);
}

void CGridWorld::fillPossibilities(CPossibleStates& o_states) 
{
    // Spawn four new states which occurs after corresponding actions and choose 
    // some according to policy
    CStatePtr ptrState = currentState();

    int curRow = ptrState->GetValue("ROW");
    int curCol = ptrState->GetValue("COLUMN");

    // Move left
    {
        CStatePtr ptrNewState = ptrState->Clone();
        ptrNewState->SetValue("COLUMN", applyColBounds(curCol - 1));
        ptrNewState->SetValue("ROW", applyRowBounds(curRow + m_wind[curCol]));
        if (!ptrState->IsEqual(ptrNewState)) {
            o_states.push_back(ptrNewState);
        }
    }

    // Move right
    {
        CStatePtr ptrNewState = ptrState->Clone();
        ptrNewState->SetValue("COLUMN", applyColBounds(curCol + 1));
        ptrNewState->SetValue("ROW", applyRowBounds(curRow + m_wind[curCol]));
        if (!ptrState->IsEqual(ptrNewState)) {
            o_states.push_back(ptrNewState);
        }
    }

    // Move top
    {
        CStatePtr ptrNewState = ptrState->Clone();
        ptrNewState->SetValue("ROW", applyRowBounds(curRow + 1 + m_wind[curCol]));
        if (!ptrState->IsEqual(ptrNewState)) {
            o_states.push_back(ptrNewState);
        }
    }

    // Move bottom
    {
        CStatePtr ptrNewState = ptrState->Clone();
        ptrNewState->SetValue("ROW", applyRowBounds(curRow - 1 + m_wind[curCol]));
        if (!ptrState->IsEqual(ptrNewState)) {
            o_states.push_back(ptrNewState);
        }
    }
}

bool CGridWorld::observeRewardImpl(double& o_reward) const 
{
    o_reward = isTerminalState(currentState()) ? 1.0 : -1.0;
    return isTerminalState(currentState());
}

tcl::rll::CVectorDbl CGridWorld::observeTerminalRewardsImpl() const
{
    std::cout << "Episode number:" << episode() << "\t" <<
        "Episode takes: " << step() << std::endl;

    tcl::rll::CVectorDbl res;
    res.push_back(1.0);
    return res;
}

bool CGridWorld::isTerminalState(CStatePtr i_ptrState) 
{
    return i_ptrState->GetValue("ROW") == 3 && 
        i_ptrState->GetValue("COLUMN") == 7;
}

int CGridWorld::selectNextAgentImpl()
{
    return 0;
}

void CGridWorld::PrintValueFunc() 
{
    std::ofstream fout("out.txt");
    CStatePtr temp = currentState()->Clone();
    for (int row = 6; row >= 0; --row) {
        temp->SetValue("ROW", row);
        for (int col = 0; col < 10; ++col) {
            temp->SetValue("COLUMN", col);
            fout.width(9);
            fout << agents()[0]->getValue(detail::translate(temp, CActionPtr(), 0)) << " ";
        }
        fout << std::endl;
    }
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
