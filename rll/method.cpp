#include "method.hpp"
#include "environment.hpp"
#include "agent.hpp"
#include "state.hpp"
#include "action.hpp"

#include "detail/logger.hpp"

#include <functional>
#include <random>

namespace tcl { namespace rll {

void CMethodBase::Run(unsigned int i_episodes) 
{
    if (!m_ptrConfig) {
        throw CRLException("m_ptrConfig member cannot be null");
    }

    g_log.Enable(m_ptrConfig->m_enableLog);

    for (unsigned int episode = 0; episode < i_episodes; ++episode) {
        processEpisode(episode);
    }

    g_log.Enable(false);
}

void CStateMethod::processEpisode(unsigned int i_episode) 
{
    m_pEnv->initEpisode();

    bool finished = false;
    unsigned int step = 0;

    // Do episode until we get terminal state
    for(bool finished = false; !finished; ++step)
    {
        // Select active agent
        int activeAgentIdx = m_pEnv->selectNextAgent();
        CAgentPtr activeAgent = m_pEnv->agents()[activeAgentIdx];

        // Get possible next states for current active agent
        CEnvState::CPossibleStates pos;
        m_pEnv->fillPossibilities(pos);
        if (pos.empty()) {
            throw CRLException("At least must be one possible next state");
        }

        // Get value for every possible state
        CValueStateMap variants;
        std::for_each(
            pos.begin()
          , pos.end()
          , [&](CEnvState::CPossibleStates::const_reference r)
            {
                double stateValue = activeAgent->getValue(
                    translate(r, CActionPtr(), activeAgentIdx)
                  );
                variants.insert(std::make_pair(stateValue, r));
            }
          );

        // Select action and therefore next state according policy 
        CValueStateMap::const_iterator iter = m_policy.Select(variants);
        m_pEnv->setCurrentState(iter->second);

        // Get reward for agent 
        double reward;
        finished = m_pEnv->observeReward(reward);

        if (finished) 
        {
            CVectorDbl terminalRewards = m_pEnv->observeTerminalRewards();
            updateValueFunctionOnTerminalImpl(terminalRewards);
        } 
        else 
            updateValueFunctionImpl(activeAgentIdx, reward);
    }
}

void CActionMethod::processEpisode(unsigned int i_episode) 
{
    m_pEnv->initEpisode();

    unsigned int step = 0;

    // Do episode until we get terminal state
    for(bool finished = false; !finished; ++step)
    {
        // Select active agent
        int activeAgentIdx = m_pEnv->selectNextAgent();
        CAgentPtr activeAgent = m_pEnv->agents()[activeAgentIdx];

        // Get possible actions for current active agent
        CEnvAction::CPossibleActions pos;
        m_pEnv->fillPossibilities(pos);
        if (pos.empty()) {
            throw CRLException("At least must be one possible next action");
        }

        // Ok get value for every possible action
        CStatePtr currentState = m_pEnv->currentState();
        CValueActionMap variants;
        std::for_each(
            pos.begin()
          , pos.end()
          , [&](CEnvAction::CPossibleActions::const_reference r) 
            {
                double actionValue = activeAgent->getValue(
                    translate(currentState, r, activeAgentIdx)
                  );
                variants.insert(std::make_pair(actionValue, r));
            }
          );

        // Select next action
        CValueActionMap::const_iterator iter = m_policy.Select(variants);

        // Set next state
        m_pEnv->doAction(iter->second);

        // Get reward for agent 
        double reward;
        finished = m_pEnv->observeReward(reward);

        if (finished) 
        {
            CVectorDbl terminalRewards = m_pEnv->observeTerminalRewards();
            updateValueFunctionOnTerminalImpl(terminalRewards);
        } 
        else 
            updateValueFunctionImpl(activeAgentIdx, reward);
    }
}

}}
