#include "lambda_method.hpp"
#include "agent.hpp"
#include "environment.hpp"

#include "detail/logger.hpp"

#include <cassert>

namespace tcl { namespace rll { 

CLambdaTD::CLambdaTD(CEnvState* i_pEnv, CConfigPtr i_ptrConfig) 
	: CStateMethod(i_pEnv, i_ptrConfig)
	, m_traces(i_pEnv->agents().size())
	, m_ptrConfig(i_ptrConfig)
{
}

void CLambdaTD::prepareUpdates(
    const CAgentPtr& i_ptrAgent
  , CTraceMap& io_agentTraces
  , const CVectorRlltPtr& i_stateForUpdate
  , double i_stateValue
  , double i_nextStateValue
  , double i_reward
  , CAgent::CUpdateList& o_updateList
  )
{
    double temp = i_reward - i_stateValue + m_ptrConfig->m_gamma * i_nextStateValue;

    // Update eligibility trace for state
	// Accumulating traces: e(s) <- e(s) + 1
	// Replacing traces: e(s) <- 1 
    // Search trace
	CTraceMap::iterator i = io_agentTraces.find(i_stateForUpdate);

    if (io_agentTraces.end() == i) 
	{
        // No trace found. Add it.
        io_agentTraces.insert(make_pair(i_stateForUpdate, 1.0));
    } 
	else 
	{
        // Trace found. Update it.
        // Accumulating or replacing traces?
		if (m_ptrConfig->m_accumulating)
            ++i->second;
        else
            i->second = 1;
    }

    // Fill update map
    // Run over all past agent states	
    for (CTraceMap::iterator i = io_agentTraces.begin(); i != io_agentTraces.end();) 
	{
        double change = m_ptrConfig->m_alpha * temp * i->second;
        o_updateList.push_back(make_pair(i->first, i_ptrAgent->getValue(i->first) + change));
        // Reduce trace
		// TODO: m_ptrConfig->m_lambda * m_ptrConfig->m_gamma we can do it on startup
		i->second *= (m_ptrConfig->m_lambda * m_ptrConfig->m_gamma);
		
		// Erase traces that become less then m_ptrConfig->m_etEpsilon
		if (i->second < m_ptrConfig->m_etEpsilon)
		{
			CTraceMap::iterator toDelete = i++;
			io_agentTraces.erase(toDelete);
		}
		else 
			++i;
    }
}

void CLambdaTD::updateValueFunctionImpl(CEnvState* i_env, int i_agentIndex, double i_reward)
{
    CVectorRlltPtr ptrState = detail::translate(i_env->currentState(), CActionPtr(), i_agentIndex);
	CVectorRlltPtr ptrPrevState = detail::translate(i_env->previousState(), CActionPtr(), i_agentIndex);

    CAgentPtr ptrAgent = i_env->agents()[i_agentIndex];
	CTraceMap& agentTraces = m_traces[i_agentIndex];

    double V = ptrAgent->getValue(ptrPrevState);
    double newV = ptrAgent->getValue(ptrState);

	CAgent::CUpdateList updates;
	prepareUpdates(ptrAgent, agentTraces, ptrPrevState, V, newV, i_reward, updates);

    g_log.Print("TD METHOD", "flushAgentRewards", updates);

    // Update value function
    ptrAgent->update(updates);
}

void CLambdaTD::updateValueFunctionOnTerminalImpl(CEnvState* i_env, const CVectorDbl& rewards)
{
	// For each agent update value function
	for(size_t agentIdx = 0; agentIdx < i_env->agents().size(); ++agentIdx) 
	{
		CVectorRlltPtr ptrState = detail::translate(i_env->currentState(), CActionPtr(), agentIdx);

		CAgentPtr ptrAgent = i_env->agents()[agentIdx];
		CTraceMap& agentTraces = m_traces[agentIdx];

		double V = ptrAgent->getValue(ptrState);
		double newV = 0;

		CAgent::CUpdateList updates;
		prepareUpdates(ptrAgent, agentTraces, ptrState, V, newV, rewards[agentIdx], updates);

		g_log.Print("TD METHOD", "flushTerminalReward", updates);

		// Update value function
		ptrAgent->update(updates);
	}
}

void CLambdaSarsa::updateValueFunctionImpl(int i_agentIndex, double i_reward)
{
}

void CLambdaSarsa::updateValueFunctionOnTerminalImpl(const CVectorDbl& rewards)
{
}

void CLambdaWatkins::updateValueFunctionImpl(int i_agentIndex, double i_reward)
{
}

void CLambdaWatkins::updateValueFunctionOnTerminalImpl(const CVectorDbl& rewards)
{
}


///** @todo Avoid repeating of calculation value function */
//CVectorRlltPtr CLambdaSarsa::updateValueFunction(
//    int i_agentIndex
//  , double i_reward
//  , bool i_terminal
//  ) 
//{
//    CVectorRlltPtr ptrStateAction = translate(
//        m_pEnv->m_ptrPrevState
//      , m_ptrPerformedAction
//      , i_agentIndex
//      );
//
//    CAgentPtr ptrAgent = m_pEnv->m_agents[i_agentIndex];
//    assert(!(ptrAgent->m_ptrPrevState == NULL && i_terminal));
//    if (ptrAgent->m_ptrPrevState == NULL) {
//        // Agent first action
//        // This is first agent action and episode still continue
//        // Here we must only to store internal state as previous agent state
//        ptrAgent->m_ptrPrevState = ptrStateAction;
//    } else {
//        // Here is repeating of value function calculation 
//        double Q = i_terminal ? m_performedValue : ptrAgent->m_ptrFunc->GetValue(ptrAgent->m_ptrPrevState);
//        double newQ = i_terminal ? 0 : m_performedValue;
//        double temp = i_reward + m_ptrConfig->m_gamma * newQ - Q;
//
//        ptrAgent->UpdateTrace(
//            i_terminal ? ptrStateAction : ptrAgent->m_ptrPrevState
//          , m_ptrConfig->m_accumulating
//          , m_ptrConfig->m_etEpsilon
//          );
//
//        CValueFunction::CUpdateList update;
//        typedef CAgent::CTracesMap CTracesMap;
//        CTracesMap& agentTraces = ptrAgent->m_traces;
//
//        // TODO: Use lambda here
//        for (CTracesMap::iterator i = agentTraces.begin(); i != agentTraces.end(); ++i) {
//            double change = m_ptrConfig->m_alpha * temp * i->second;
//            update.push_back(make_pair(i->first, ptrAgent->m_ptrFunc->GetValue(i->first) + change));
//            // Reduce trace
//            i->second *= m_ptrConfig->m_lambda;
//        }
//        g_log.Print("SARSA METHOD", "flushAgentRewards", update);
//
//        // Update value function
//        ptrAgent->m_ptrFunc->Update(update);
//    }
//    return ptrStateAction;
//}
//
//CVectorRlltPtr CLambdaWatkins::updateValueFunction(int i_agentIndex,
//                                                  double i_reward, 
//                                                  bool i_terminal) 
//{
//    CVectorRlltPtr ptrStateAction = translate(
//        m_pEnv->m_ptrPrevState
//      , m_ptrPerformedAction
//      , i_agentIndex
//      );
//    CAgentPtr ptrAgent = m_pEnv->m_agents[i_agentIndex];
//    assert(!(ptrAgent->m_ptrPrevState == NULL && i_terminal));
//    if (ptrAgent->m_ptrPrevState == NULL) {
//        // Agent first action
//        // This is first agent action and episode still continue
//        // Here we must only to store internal state as previous agent state
//        ptrAgent->m_ptrPrevState = ptrStateAction;
//    } else {
//        double Q = i_terminal ? m_performedValue : ptrAgent->m_ptrFunc->GetValue(ptrAgent->m_ptrPrevState);
//        // More complicated rule for newQ than in Sarsa
//        double newQ = i_terminal ? 
//            0 : (m_performedValue == m_greedyValue ? m_performedValue : m_greedyValue);
//        double temp = i_reward + m_ptrConfig->m_gamma * newQ - Q;
//
//        // Add previous state to eligibility traces 
//        ptrAgent->UpdateTrace(
//            i_terminal ? ptrStateAction : ptrAgent->m_ptrPrevState
//          , m_ptrConfig->m_accumulating
//          , m_ptrConfig->m_etEpsilon
//          );
//
//        CValueFunction::CUpdateList update;
//        typedef CAgent::CTracesMap CTracesMap;
//        CTracesMap& agentTraces = ptrAgent->m_traces;
//
//        // Use lambda here
//        for (CTracesMap::iterator i = agentTraces.begin(); i != agentTraces.end(); ++i) {
//            double change = m_ptrConfig->m_alpha * temp * i->second;
//            update.push_back(make_pair(i->first, ptrAgent->m_ptrFunc->GetValue(i->first) + change));
//            // Reduce trace
//            i->second *= m_ptrConfig->m_lambda;
//        }
//        g_log.Print("WATKINS METHOD", "flushAgentRewards", update);
//        // If the last action was not greedy then kill all traces
//        if (!i_terminal && (m_performedValue != m_greedyValue)) {
//            agentTraces.clear();
//        }
//        ptrAgent->m_ptrFunc->Update(update);
//    }
//    return ptrStateAction;
//}

}}