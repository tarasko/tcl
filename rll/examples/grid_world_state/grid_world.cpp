#include "grid_world.hpp"
#include <tcl/rll/lambda_method.hpp>
#include <tcl/rll/lookup_table.hpp>

#include <map>
#include <fstream>
#include <iostream>

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

std::vector<CStatePtr> CGridWorld::getPossibleNextStates() const
{
    std::vector<CStatePtr> result;

    int curRow = m_state->GetValue("ROW");
    int curCol = m_state->GetValue("COLUMN");

    // Move left
    {
        CStatePtr ptrNewState = m_state->Clone();
        ptrNewState->SetValue("COLUMN", applyColBounds(curCol - 1));
        ptrNewState->SetValue("ROW", applyRowBounds(curRow + m_wind.at(curCol)));
        if (!m_state->IsEqual(ptrNewState)) {
            result.push_back(ptrNewState);
        }
    }

    // Move right
    {
        CStatePtr ptrNewState = m_state->Clone();
        ptrNewState->SetValue("COLUMN", applyColBounds(curCol + 1));
        ptrNewState->SetValue("ROW", applyRowBounds(curRow + m_wind.at(curCol)));
        if (!m_state->IsEqual(ptrNewState)) {
            result.push_back(ptrNewState);
        }
    }

    // Move top
    {
        CStatePtr ptrNewState = m_state->Clone();
        ptrNewState->SetValue("ROW", applyRowBounds(curRow + 1 + m_wind.at(curCol)));
        if (!m_state->IsEqual(ptrNewState)) {
            result.push_back(ptrNewState);
        }
    }

    // Move bottom
    {
        CStatePtr ptrNewState = m_state->Clone();
        ptrNewState->SetValue("ROW", applyRowBounds(curRow - 1 + m_wind.at(curCol)));
        if (!m_state->IsEqual(ptrNewState)) {
            result.push_back(ptrNewState);
        }
    }

    return result;
}

bool CGridWorld::setNextStateAssignRewards(const CStatePtr& state)
{
    m_state = state;
    if (isTerminalState(state))
    {
        std::cout << "Episode number:" << episode() << "\t" <<
            "Episode takes: " << step() << std::endl;

        agents()[0]->addReward(1.0);
        return false;
    }
    else
    {
        agents()[0]->addReward(-1.0);
        return true;
    }
}

bool CGridWorld::isTerminalState(CStatePtr i_ptrState) 
{
    return 
        i_ptrState->GetValue("ROW") == 3 && 
        i_ptrState->GetValue("COLUMN") == 7;
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
