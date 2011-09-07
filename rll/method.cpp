#include "method.hpp"
#include "environment.hpp"
#include "agent.hpp"
#include "state.hpp"
#include "action.hpp"

#include "detail/logger.hpp"
#include "detail/utils.hpp"

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
    int activeAgentIdx;
    
    // Repeat for each step in episode
    do
    {
        // Get active agent on this step
        activeAgentIdx = m_pEnv->activeAgent();
        CAgentPtr activeAgent = m_pEnv->agents()[activeAgentIdx];

        // Get possible next states for current active agent
        auto nextStates = m_pEnv->getPossibleNextStatesImpl();
        if (nextStates.empty()) {
            throw CRLException("at least must be one possible next state");
        }

        // Get value for every possible state
        CValueStateMap variants;
        std::for_each(
            nextStates.begin()
          , nextStates.end()
          , [&](const CStatePtr& state)
            {
                double stateValue = activeAgent->getValue(
                    detail::translate(state, CActionPtr(), activeAgentIdx)
                  );
                variants.insert(std::make_pair(stateValue, state));
            }
          );

        // Select action and therefore next state according policy 
        CValueStateMap::const_iterator iter = m_policy.Select(variants);

        // Get reward for agent 
        double reward = m_pEnv->setNextStateObserveRewardImpl(iter->second);

        // Update value function
        updateValueFunctionImpl(activeAgent, activeAgentIdx, iter->second, reward);

        // Remember new state for active agent
        activeAgent->setLastStateWhenWasActive(iter->second);
    } while(m_pEnv->nextStep());

    // Observe terminal rewards for all agents, update value function
    CVectorDbl terminalRewards = m_pEnv->observeTerminalRewards();
    terminalRewards[activeAgentIdx] = 0.0;

    // Update value function for last state for each agent according to terminal rewards
    for(size_t agentIdx = 0; agentIdx < terminalRewards.size(); ++agentIdx) 
    {
        updateValueFunctionImpl(
            m_pEnv->agents()[agentIdx]
          , agentIdx
          , CStatePtr()
          , terminalRewards[agentIdx]
          );
    }
}

void CActionMethod::processEpisode(unsigned int i_episode) 
{
    //m_pEnv->initEpisode();
    //int activeAgentIdx;
    //
    //CActionPtr action = selectActionForStateAccordingPolicy();

    //// Repeat for each step in episode
    //do
    //{
    //    // Select active agent
    //    activeAgentIdx = m_pEnv->activeAgent();
    //    CAgentPtr activeAgent = m_pEnv->agents()[activeAgentIdx];

    //    // Get possible actions for current active agent
    //    CStatePtr nextState = m_pEnv->evaluateNextState();
    //    auto possibleActionsFromNextState = m_pEnv->getPossibleActions();
    //    if (possibleActions.empty()) {
    //        throw CRLException("At least must be one possible next action");
    //    }

    //    // Ok get value for every possible action
    //    CStatePtr currentState = m_pEnv->currentState();
    //    CValueActionMap variants;
    //    std::for_each(
    //        pos.begin()
    //      , pos.end()
    //      , [&](CEnvAction::CPossibleActions::const_reference r) 
    //        {
    //            double actionValue = activeAgent->getValue(
    //                detail::translate(currentState, r, activeAgentIdx)
    //              );
    //            variants.insert(std::make_pair(actionValue, r));
    //        }
    //      );

    //    // Select next action
    //    CValueActionMap::const_iterator iter = m_policy.Select(variants);

    //    // Set next state
    //    m_pEnv->doAction(iter->second);

    //    // Get reward for agent 
    //    double reward;
    //    finished = m_pEnv->observeReward(reward);
    //    updateValueFunctionImpl(activeAgentIdx, reward);

    //    if (finished) 
    //    {
    //        CVectorDbl terminalRewards = m_pEnv->observeTerminalRewards();
    //        updateValueFunctionOnTerminalImpl(terminalRewards);
    //    } 
    //}
}

}}
