#include "lambda_method.hpp"
#include "detail/logger.hpp"

#include <cassert>

namespace tcl { namespace rll { 

CVectorRlltPtr CLambdaTD::getPreviousState(int i_agentIndex) 
{
    CAgentPtr ptrAgent = m_pEnv->m_agents[i_agentIndex];
    if (ptrAgent->m_ptrPrevState != NULL) {
        return ptrAgent->m_ptrPrevState;
    }

    // IMPORTANT: Make copy of current state
    assert(m_pEnv != NULL);
    ptrAgent->m_ptrPrevState = translate(m_pEnv->m_ptrPrevState, CActionPtr(), i_agentIndex);
    return ptrAgent->m_ptrPrevState;
}

CVectorRlltPtr CLambdaTD::updateValueFunction(
    int i_agentIndex
  , double i_reward
  , bool i_terminal
  ) 
{
    CVectorRlltPtr ptrState = translate(m_pEnv->m_ptrState, CActionPtr(), i_agentIndex);
    CVectorRlltPtr ptrPrevState = getPreviousState(i_agentIndex);

    CAgentPtr ptrAgent = m_pEnv->m_agents[i_agentIndex];

    // temp = (reward + gamma * newV) - oldV
    double V = ptrAgent->m_ptrFunc->GetValue(ptrPrevState);
    double newV = i_terminal ? 0 : m_pEnv->m_cache;
    double temp = i_reward - V + m_ptrConfig->m_gamma * newV;

    // Update eligibility trace for state
    // e(s) <- e(s) + 1
    ptrAgent->UpdateTrace(
        ptrPrevState
      , m_ptrConfig->m_accumulating
      , m_ptrConfig->m_etEpsilon
      );

    // Fill update map
    // Run over all past agent states
    typedef CAgent::CTracesMap CTracesMap;
    CTracesMap& agentTraces = ptrAgent->m_traces;
    CValueFunction::CUpdateList update;
    // TODO: Use lambdas
    for (CTracesMap::iterator i = agentTraces.begin(); i != agentTraces.end(); ++i) {
        double change = m_ptrConfig->m_alpha * temp * i->second;
        update.push_back(make_pair(i->first, ptrAgent->m_ptrFunc->GetValue(i->first) + change));
        // Reduce trace
        i->second *= m_ptrConfig->m_lambda;
    }

    // Special case for rewarding terminal state
    if (m_ptrConfig->m_rewardTerminal && i_terminal) {
        double newV = m_pEnv->m_cache;
        double change = m_ptrConfig->m_alpha * (i_reward - newV);
        update.push_back(make_pair(ptrState, newV + change));
    }

    g_log.Print("TD METHOD", "flushAgentRewards", update);

    // Update value function
    ptrAgent->m_ptrFunc->Update(update);

    return ptrState;
}

/** @todo Avoid repeating of calculation value function */
CVectorRlltPtr CLambdaSarsa::updateValueFunction(
    int i_agentIndex
  , double i_reward
  , bool i_terminal
  ) 
{
    CVectorRlltPtr ptrStateAction = translate(
        m_pEnv->m_ptrPrevState
      , m_ptrPerformedAction
      , i_agentIndex
      );

    CAgentPtr ptrAgent = m_pEnv->m_agents[i_agentIndex];
    assert(!(ptrAgent->m_ptrPrevState == NULL && i_terminal));
    if (ptrAgent->m_ptrPrevState == NULL) {
        // Agent first action
        // This is first agent action and episode still continue
        // Here we must only to store internal state as previous agent state
        ptrAgent->m_ptrPrevState = ptrStateAction;
    } else {
        // Here is repeating of value function calculation 
        double Q = i_terminal ? m_performedValue : ptrAgent->m_ptrFunc->GetValue(ptrAgent->m_ptrPrevState);
        double newQ = i_terminal ? 0 : m_performedValue;
        double temp = i_reward + m_ptrConfig->m_gamma * newQ - Q;

        ptrAgent->UpdateTrace(
            i_terminal ? ptrStateAction : ptrAgent->m_ptrPrevState
          , m_ptrConfig->m_accumulating
          , m_ptrConfig->m_etEpsilon
          );

        CValueFunction::CUpdateList update;
        typedef CAgent::CTracesMap CTracesMap;
        CTracesMap& agentTraces = ptrAgent->m_traces;

        // TODO: Use lambda here
        for (CTracesMap::iterator i = agentTraces.begin(); i != agentTraces.end(); ++i) {
            double change = m_ptrConfig->m_alpha * temp * i->second;
            update.push_back(make_pair(i->first, ptrAgent->m_ptrFunc->GetValue(i->first) + change));
            // Reduce trace
            i->second *= m_ptrConfig->m_lambda;
        }
        g_log.Print("SARSA METHOD", "flushAgentRewards", update);

        // Update value function
        ptrAgent->m_ptrFunc->Update(update);
    }
    return ptrStateAction;
}

CVectorRlltPtr CLambdaWatkins::updateValueFunction(int i_agentIndex,
                                                  double i_reward, 
                                                  bool i_terminal) 
{
    CVectorRlltPtr ptrStateAction = translate(
        m_pEnv->m_ptrPrevState
      , m_ptrPerformedAction
      , i_agentIndex
      );
    CAgentPtr ptrAgent = m_pEnv->m_agents[i_agentIndex];
    assert(!(ptrAgent->m_ptrPrevState == NULL && i_terminal));
    if (ptrAgent->m_ptrPrevState == NULL) {
        // Agent first action
        // This is first agent action and episode still continue
        // Here we must only to store internal state as previous agent state
        ptrAgent->m_ptrPrevState = ptrStateAction;
    } else {
        double Q = i_terminal ? m_performedValue : ptrAgent->m_ptrFunc->GetValue(ptrAgent->m_ptrPrevState);
        // More complicated rule for newQ than in Sarsa
        double newQ = i_terminal ? 
            0 : (m_performedValue == m_greedyValue ? m_performedValue : m_greedyValue);
        double temp = i_reward + m_ptrConfig->m_gamma * newQ - Q;

        // Add previous state to eligibility traces 
        ptrAgent->UpdateTrace(
            i_terminal ? ptrStateAction : ptrAgent->m_ptrPrevState
          , m_ptrConfig->m_accumulating
          , m_ptrConfig->m_etEpsilon
          );

        CValueFunction::CUpdateList update;
        typedef CAgent::CTracesMap CTracesMap;
        CTracesMap& agentTraces = ptrAgent->m_traces;

        // Use lambda here
        for (CTracesMap::iterator i = agentTraces.begin(); i != agentTraces.end(); ++i) {
            double change = m_ptrConfig->m_alpha * temp * i->second;
            update.push_back(make_pair(i->first, ptrAgent->m_ptrFunc->GetValue(i->first) + change));
            // Reduce trace
            i->second *= m_ptrConfig->m_lambda;
        }
        g_log.Print("WATKINS METHOD", "flushAgentRewards", update);
        // If the last action was not greedy then kill all traces
        if (!i_terminal && (m_performedValue != m_greedyValue)) {
            agentTraces.clear();
        }
        ptrAgent->m_ptrFunc->Update(update);
    }
    return ptrStateAction;
}

}}