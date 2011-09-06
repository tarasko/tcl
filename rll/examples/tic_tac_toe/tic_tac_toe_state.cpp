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

    virtual void initEpisodeImpl();
    virtual int selectNextAgentImpl();
    virtual void fillPossibilities(CPossibleStates& o_states);
    virtual bool observeRewardImpl(double& o_reward) const;
    virtual CVectorDbl observeTerminalRewardsImpl() const;

    /// @param o_reward - Get reward for current state.
    /// @return true if game has finished.
    bool analyzeState(CVectorDbl& o_rewards) const;
    void printState() const;

private:
    int m_activeAgentIdx;
    mutable CVectorDbl m_lastTerminalRewards;
};

CTicTacToeState::CTicTacToeState() 
{
    CStatePtr ptrState = make_shared<CState>();
    ptrState->RegisterVariable("00");
    ptrState->RegisterVariable("01");
    ptrState->RegisterVariable("02");
    ptrState->RegisterVariable("10");
    ptrState->RegisterVariable("11");
    ptrState->RegisterVariable("12");
    ptrState->RegisterVariable("20");
    ptrState->RegisterVariable("21");
    ptrState->RegisterVariable("22");
    setCurrentState(ptrState);

    // Create value function and method
    CValueFunctionPtr ptrFunc = make_shared<CLookupTable>();
    CAgentPtr XPlayer = make_shared<CAgent>(ptrFunc);
    CAgentPtr OPlayer = make_shared<CAgent>(ptrFunc);
    agents().push_back(XPlayer);
    agents().push_back(OPlayer);
}

void CTicTacToeState::initEpisodeImpl() 
{
    m_activeAgentIdx = 1;
    for (int x=0; x<3; ++x) 
    {
        for (int y=0; y<3; ++y) 
            currentState()->SetValue(formatState(x, y), 0);
    }
}

int CTicTacToeState::selectNextAgentImpl()
{
    m_activeAgentIdx = m_activeAgentIdx == 0 ? 1 : 0;
    return m_activeAgentIdx;
}

void CTicTacToeState::fillPossibilities(CPossibleStates& o_states) 
{
    // Run over all squares
    int squares[3][3];
    for (int x=0; x<3; ++x) 
    {
        for (int y=0; y<3; ++y) 
        {
            // Fill squares
            const char* dest = formatState(x, y);
            squares[x][y] = currentState()->GetValue(dest);
            if (0 == squares[x][y]) 
            {
                // Make new state
                CStatePtr ptrNewState = currentState()->Clone();
                ptrNewState->SetValue(dest, m_activeAgentIdx == 0 ? 1 : 2);
                o_states.push_back(ptrNewState);
            }
        }
    }
}

bool CTicTacToeState::observeRewardImpl(double& o_reward) const
{
    if ((episode() % 100) == 0) 
        printState();

    CVectorDbl rewards(2);
    bool finished = analyzeState(rewards);
    o_reward = rewards[m_activeAgentIdx];

    if (finished) 
        m_lastTerminalRewards = std::move(rewards);

    return finished;
}

CVectorDbl CTicTacToeState::observeTerminalRewardsImpl() const
{
    return std::move(m_lastTerminalRewards);
}

bool CTicTacToeState::analyzeState(CVectorDbl& o_rewards) const
{
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
            squares[x][y] = currentState()->GetValue(formatState(x, y));
            // Fill my
            my[x][y] = squares[x][y] == activePlayerSigns;
            // Update noEmptySquares
            hasEmptySquares |= squares[x][y] == 0;
        }
    }

    auto assignRewards = [](CVectorDbl& o_rewards, int i_whoWin) -> void
    {
        if (i_whoWin == 0) 
        {
            o_rewards[0] = 1.0;
            o_rewards[1] = -1.0;
        } 
        else 
        {
            o_rewards[1] = 1.0;
            o_rewards[0] = -1.0;
        }
    };

    // Check horizontal lines
    for (int y=0; y<3; ++y) {
        bool lastMoveWin = squares[0][y] == squares[1][y] && 
            squares[1][y] == squares[2][y] &&
            squares[0][y] == activePlayerSigns;
        if (lastMoveWin) {
            assignRewards(o_rewards, m_activeAgentIdx);
            return true;
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
            assignRewards(o_rewards, m_activeAgentIdx);
            return true;
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
        assignRewards(o_rewards, m_activeAgentIdx);
        return true;
    }

    // Ok now check for draw
    if (!hasEmptySquares) 
        return true;

    return false;
}

void CTicTacToeState::printState() const
{
    cout << "Episode: " << episode()
        << " Step: " <<  step() 
        << " Value: " 
        << agents()[m_activeAgentIdx]->getValue(detail::translate(currentState(), CActionPtr(), m_activeAgentIdx))
        << endl;

    for (int x=0; x<3; ++x) {
        for (int y=0; y<3; ++y) {
            const char* dest = formatState(x, y);
            // Fill squares
            int sign = currentState()->GetValue(dest);
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

    CTicTacToeState game;
    CLambdaTD m(&game, ptrConfig);

    m.Run(100000);
}
