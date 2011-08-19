#include <tcl/rll/environment.hpp>
#include <tcl/rll/neuronal_network.hpp>
#include <tcl/rll/lookup_table.hpp>
#include <tcl/rll/lambda_method.hpp>

#include <vector>
#include <fstream>
#include <iostream>
#include <boost/any.hpp>
#include <boost/lexical_cast.hpp>

using namespace tcl::rll;
using boost::any_cast;
using boost::lexical_cast;
using namespace std;

class CTicTacToeState : public CEnvState {
public:
  CTicTacToeState(void) {
    // Create state
    m_ptrState.reset(new CState());
    m_ptrState->RegisterVariable("00");
    m_ptrState->RegisterVariable("01");
    m_ptrState->RegisterVariable("02");
    m_ptrState->RegisterVariable("10");
    m_ptrState->RegisterVariable("11");
    m_ptrState->RegisterVariable("12");
    m_ptrState->RegisterVariable("20");
    m_ptrState->RegisterVariable("21");
    m_ptrState->RegisterVariable("22");

    // Create value function and method
    CValueFunctionPtr ptrFunc(new CLookupTable);
    CAgentPtr XPlayer(new CAgent(ptrFunc));
    CAgentPtr OPlayer(new CAgent(ptrFunc));
    m_agents.push_back(XPlayer);
    m_agents.push_back(OPlayer);
  }

protected:
  void                  initEpisode() {
    for (int x=0; x<3; ++x) {
      string sx = boost::lexical_cast<string, int>(x);
      for (int y=0; y<3; ++y) {
        string sy = boost::lexical_cast<string, int>(y);
        string dest = sx + sy;
        m_ptrState->SetValue(dest.c_str(), 0);
      }
    }
    m_activeAgent = 0;
  }

  virtual bool          isEpisodeFinished() {
    CVectorDbl temp(2, 0.0);
    return analyzeState(temp);
  }

  virtual void          fillPossibilities(CPossibleStates& o_states) {
    // Run over all squares
    int squares[3][3];
    for (int x=0; x<3; ++x) {
      string sx = lexical_cast<string, int>(x);
      for (int y=0; y<3; ++y) {
        string sy = lexical_cast<string, int>(y);
        string dest = sx + sy;
        // Fill squares
        squares[x][y] = any_cast<int>(m_ptrState->GetValue(dest.c_str()));
        if (0 == squares[x][y]) {
          // Make new state
          CStatePtr ptrNewState = m_ptrState->Clone();
          ptrNewState->SetValue(dest.c_str(), 
            m_activeAgent == 0 ? 1 : 2);
          o_states.push_back(ptrNewState);
        }
      }
    }
  }

  virtual void          observeRewards(CVectorDbl& o_rewards) {
    if ((m_episode % 100) == 0) {
      printState();
    }
    analyzeState(o_rewards);
  }

  /** @param o_reward - Get reward for current state
    @return true if game has finished */
  bool                  analyzeState(CVectorDbl& o_rewards) {
    // 0-empty 1-X 2-O 
    int   squares[3][3];
    // true - if player that just has maked move has it own sign
    // in corresponding square
    bool  my[3][3];
    // true if there are no empty squares
    bool  hasEmptySquares = false;
    int   activePlayerSigns = m_activeAgent == 0 ? 1 : 2;

    struct FRewardIt {
      void operator() (CVectorDbl& o_rewards, int i_whoWin) {
        if (i_whoWin == 0) {
          o_rewards[0] = 1.0;
          o_rewards[1] = -1.0;
        } else {
          o_rewards[1] = 1.0;
          o_rewards[0] = -1.0;
        }
      }
    } RewardIt;

    // Fill arrays
    for (int x=0; x<3; ++x) {
      string sx = lexical_cast<string, int>(x);
      for (int y=0; y<3; ++y) {
        string sy = lexical_cast<string, int>(y);
        string dest = sx + sy;
        // Fill squares
        squares[x][y] = any_cast<int>(m_ptrState->GetValue(dest.c_str()));
        // Fill my
        my[x][y] = squares[x][y] == activePlayerSigns;
        // Update noEmptySquares
        hasEmptySquares |= squares[x][y] == 0;
      }
    }
    // Check horizontal lines
    for (int y=0; y<3; ++y) {
      bool last_move_win = squares[0][y] == squares[1][y] && 
                           squares[1][y] == squares[2][y] &&
                           squares[0][y] == activePlayerSigns;
      if (last_move_win) {
        RewardIt(o_rewards, m_activeAgent);
        return true;
      }
    }

    // Check vertical lines
    for (int x=0; x<3; ++x) {
      bool last_move_win = squares[x][0] == squares[x][1] && 
                           squares[x][1] == squares[x][2] &&
                           squares[x][0] == activePlayerSigns;
      if (last_move_win) {
        RewardIt(o_rewards, m_activeAgent);
        return true;
      }
    }

    // Check diagonal lines
    bool last_move_win = squares[0][0] == squares[1][1] &&
                         squares[1][1] == squares[2][2] &&
                         squares[0][0] == activePlayerSigns;
    last_move_win |= squares[2][0] == squares[1][1] &&
                     squares[1][1] == squares[0][2] &&
                     squares[2][0] == activePlayerSigns;
    if (last_move_win) {
        RewardIt(o_rewards, m_activeAgent);
        return true;
    }

    // Ok now check for draw
    if (!hasEmptySquares) {
      return true;
    }

    return false;
  }

  virtual void          selectNextAgent() {
    m_activeAgent = m_activeAgent == 0 ? 1 : 0;
  }

  void                  printState() {
    cout << "Episode: " << m_episode
      << " Step: " <<  m_step 
      << " Value: " 
      << m_agents[0]->m_ptrFunc->GetValue(m_ptrState, CActionPtr(), m_activeAgent)
      << endl;
    for (int x=0; x<3; ++x) {
      string sx = lexical_cast<string, int>(x);
      for (int y=0; y<3; ++y) {
        string sy = lexical_cast<string, int>(y);
        string dest = sx + sy;
        // Fill squares
        int sign = any_cast<int>(m_ptrState->GetValue(dest.c_str()));
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

};

int main() {
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
