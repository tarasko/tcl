#include <tcl/rll/rll.hpp>
#include <tcl/rll/detail/utils.hpp>

#include <vector>
#include <fstream>
#include <iostream>

using namespace tcl::rll;
using namespace std;

class CTicTacToeState : public CEnvState 
{
public:
    CTicTacToeState();

private:
    static const char* formatState(int x, int y)
    {
        static char buf[3];
        sprintf(buf, "%d%d", x, y);
        return buf;
    }

	virtual void initEpisode();
    virtual size_t activeAgent() const;
    virtual CStatePtr currentState() const;
    virtual std::vector<CStatePtr> getPossibleNextStates() const;
    virtual bool setNextStateAssignRewards(const CStatePtr& state);


    /// @param o_reward - Get reward for current state.
    /// @return true if game has finished.
    bool analyzeState(CVectorDbl& o_rewards) const;
    void printState() const;

private:
    size_t    m_activeAgentIdx;
    CStatePtr m_state;
};

CTicTacToeState::CTicTacToeState() 
{
    m_state = make_shared<CState>();
    m_state->RegisterVariable("00");
    m_state->RegisterVariable("01");
    m_state->RegisterVariable("02");
    m_state->RegisterVariable("10");
    m_state->RegisterVariable("11");
    m_state->RegisterVariable("12");
    m_state->RegisterVariable("20");
    m_state->RegisterVariable("21");
    m_state->RegisterVariable("22");

    // Create value function and method
    CValueFunctionPtr ptrFunc = make_shared<CLookupTable>();
    CAgentPtr XPlayer = make_shared<CAgent>(ptrFunc);
    CAgentPtr OPlayer = make_shared<CAgent>(ptrFunc);
    agents().push_back(XPlayer);
    agents().push_back(OPlayer);
}

void CTicTacToeState::initEpisode() 
{
    m_activeAgentIdx = 0;
    for (int x=0; x<3; ++x) 
    {
        for (int y=0; y<3; ++y) 
            m_state->SetValue(formatState(x, y), 0);
    }
}

size_t CTicTacToeState::activeAgent() const
{
    return m_activeAgentIdx;
}

CStatePtr CTicTacToeState::currentState() const
{
    return m_state;
}

std::vector<CStatePtr> CTicTacToeState::getPossibleNextStates() const
{
    std::vector<CStatePtr> result;

    // Run over all squares
    int squares[3][3];
    for (int x=0; x<3; ++x) 
    {
        for (int y=0; y<3; ++y) 
        {
            // Fill squares
            const char* dest = formatState(x, y);
            squares[x][y] = m_state->GetValue(dest);
            if (0 == squares[x][y]) 
            {
                // Make new state
                CStatePtr ptrNewState = m_state->Clone();
                ptrNewState->SetValue(dest, m_activeAgentIdx == 0 ? 1 : 2);
                result.push_back(ptrNewState);
            }
        }
    }

    return result;
}

bool CTicTacToeState::setNextStateAssignRewards(const CStatePtr& state)
{
    m_state = state;

    // 0-empty 1-X 2-O 
    int   squares[3][3];
    // true - if player that just has maked move has it own sign
    // in corresponding square
    bool  my[3][3];
    // true if there are no empty squares
    bool  hasEmptySquares = false;
    int   activePlayerSigns = m_activeAgentIdx == 0 ? 1 : 2;

    // Fill arrays
    for (int x=0; x<3; ++x) {
        for (int y=0; y<3; ++y) {
            // Fill squares
            squares[x][y] = m_state->GetValue(formatState(x, y));
            // Fill my
            my[x][y] = squares[x][y] == activePlayerSigns;
            // Update noEmptySquares
            hasEmptySquares |= squares[x][y] == 0;
        }
    }

    // Check horizontal lines
    for (int y=0; y<3; ++y) {
        bool lastMoveWin = squares[0][y] == squares[1][y] && 
            squares[1][y] == squares[2][y] &&
            squares[0][y] == activePlayerSigns;
        if (lastMoveWin) {
            agents()[m_activeAgentIdx]->addReward(1.0);
            agents()[m_activeAgentIdx ^ 1]->addReward(-1.0);
            return false;
        }
    }

    // Check vertical lines
    for (int x=0; x<3; ++x) 
    {
        bool lastMoveWin = 
            squares[x][0] == squares[x][1] && 
            squares[x][1] == squares[x][2] &&
            squares[x][0] == activePlayerSigns;

        if (lastMoveWin) 
        {
            agents()[m_activeAgentIdx]->addReward(1.0);
            agents()[m_activeAgentIdx ^ 1]->addReward(-1.0);
            return false;
        }
    }

    // Check diagonal lines
    bool lastMoveWin = 
        squares[0][0] == squares[1][1] &&
        squares[1][1] == squares[2][2] &&
        squares[0][0] == activePlayerSigns;

    lastMoveWin |= 
        squares[2][0] == squares[1][1] &&
        squares[1][1] == squares[0][2] &&
        squares[2][0] == activePlayerSigns;

    if (lastMoveWin) 
    {
        agents()[m_activeAgentIdx]->addReward(1.0);
        agents()[m_activeAgentIdx ^ 1]->addReward(-1.0);
        return false;
    }

    // Ok now check for draw
    if (!hasEmptySquares) 
        return false;

    // Switch active agent
    printState();
    m_activeAgentIdx ^= 1;

    return true;
}

void CTicTacToeState::printState() const
{
    if (episode() % 100) 
        return;

    cout << "Episode: " << episode()
        << " Step: " <<  step() 
        << " Value: " 
        << agents()[m_activeAgentIdx]->getValue(detail::translate(currentState(), CActionPtr(), m_activeAgentIdx))
        << endl;

    for (int x=0; x<3; ++x) {
        for (int y=0; y<3; ++y) {
            const char* dest = formatState(x, y);
            // Fill squares
            int sign = m_state->GetValue(dest);
            if (1 == sign) {
                cout << "X";
            } else if (2 == sign) {
                cout << "O";
            } else {
                cout << " ";
            }
            cout << " ";
        }
        cout << endl;
    }
}


int main() 
{
    CConfigPtr ptrConfig(new CConfig);
    ptrConfig->m_alpha = 0.1;
    ptrConfig->m_lambda = 0.5;
    ptrConfig->m_gamma = 1.0;
    ptrConfig->m_accumulating = false;
    ptrConfig->m_hidden = 8;
    ptrConfig->m_vfMin = -1.0;
    ptrConfig->m_vfMax = 1.0;
    ptrConfig->m_policy = CConfig::EPSILON_GREEDY;

    CTicTacToeState game;
    CLambdaTD m(&game, ptrConfig);

    m.run(100000);
}
