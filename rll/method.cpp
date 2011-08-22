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
    m_pEnv->m_episode = i_episode;
    m_pEnv->m_step = 0;
    m_pEnv->initEpisode();

    // Ok, we got correct m_ptrState in environment. Now reset agents.
    for_each(
        m_pEnv->m_agents.begin()
      , m_pEnv->m_agents.end()
      , std::mem_fn(&CAgent::Clean)
      );

    bool finished = false;
    do {
        // Get possible next states
        CEnvState::CPossibleStates pos;
        m_pEnv->fillPossibilities(pos);
        if (pos.empty()) {
            throw CRLException("At least must be one possible next state");
        }

        // SELECT NEXT STATE
        // Ok get value for every possible state
        CValueStateMap variants;
        CAgentPtr activeAgent = m_pEnv->m_agents[m_pEnv->m_activeAgent];
        std::for_each(
            pos.begin()
          , pos.end()
          , [&](CEnvState::CPossibleStates::const_reference r) 
            {
                double stateValue = activeAgent->m_ptrFunc->GetValue(
                    translate(r, CActionPtr(), m_pEnv->m_activeAgent)
                  );
                variants.insert(std::make_pair(stateValue, r));
            }
          );

        // Save current state
        m_pEnv->m_ptrPrevState = m_pEnv->m_ptrState;
        // Select action and therefore next state according policy 
        CValueStateMap::const_iterator iter = m_policy.Select(variants);
        m_pEnv->m_cache = iter->first;
        m_pEnv->m_ptrState = iter->second;

        finished = m_pEnv->isEpisodeFinished();

        // GET REWARDS FOR EVERY AGENT FOR NEXT STATE
        CVectorDbl rewards(m_pEnv->m_agents.size(), rll_type(0.0));
        m_pEnv->observeRewards(rewards);
        if (rewards.size() != m_pEnv->m_agents.size()) {
            throw CRLException("Rewards vector must be with size equal to number of agents");
        }

        // UPDATE VALUE FUNCTION
        // Accumulate agents rewards
        for (CVectorDbl::size_type i = 0; i < rewards.size() ; ++i) {
            m_pEnv->m_agents[i]->m_reward += rewards[i];
        }

        if (m_pEnv->isEpisodeFinished()) {
            for (std::vector<CAgentPtr>::size_type i=0; i < m_pEnv->m_agents.size(); ++i) {
                double reward = m_pEnv->m_agents[i]->m_reward;
                m_pEnv->m_agents[i]->m_reward = 0.0;
                CVectorRlltPtr ptrAgentState = updateValueFunction(
                    static_cast<int>(i), 
                    reward, 
                    true);
                if (m_pEnv->m_activeAgent == i) {
                    m_pEnv->m_agents[i]->m_ptrPrevState = ptrAgentState;
                }
            }
        } else {
            double reward = m_pEnv->m_agents[m_pEnv->m_activeAgent]->m_reward;
            m_pEnv->m_agents[m_pEnv->m_activeAgent]->m_reward = 0.0;
            m_pEnv->m_agents[m_pEnv->m_activeAgent]->m_ptrPrevState = 
                updateValueFunction(m_pEnv->m_activeAgent, reward, false);
        }

        // Change active agent
        m_pEnv->selectNextAgent();

        ++m_pEnv->m_step;
    } while (!finished);
}

void CActionMethod::performAgentUpdate(const CVectorDbl& i_rewards) 
{
    CAgentPtr ptrAgent = m_pEnv->m_agents[m_pEnv->m_activeAgent];
    double reward = ptrAgent->m_reward;
    ptrAgent->m_reward = 0.0;
    ptrAgent->m_ptrPrevState = updateValueFunction(m_pEnv->m_activeAgent, reward, false);

    // Accumulate agents rewards
    for (CVectorDbl::size_type i = 0; i < i_rewards.size() ; ++i) {
        m_pEnv->m_agents[i]->m_reward += i_rewards[i];
    }
}

void CActionMethod::processEpisode(unsigned int i_episode) 
{
    m_pEnv->m_episode = i_episode;
    m_pEnv->m_step = 0;
    m_pEnv->initEpisode();

    // Ok, we got correct m_ptrState in environment. Now reset agents.
    std::for_each(
        m_pEnv->m_agents.begin()
      , m_pEnv->m_agents.end()
      , std::mem_fn(&CAgent::Clean)
      );

    bool finished = false;

    // Get possible actions
    do {
        CEnvAction::CPossibleActions pos;
        m_pEnv->fillPossibilities(pos);
        if (pos.empty()) {
            throw CRLException("At least must be one possible next action");
        }

        // SELECT NEXT ACTION
        // Ok get value for every possible action
        CValueActionMap variants;
        CAgentPtr activeAgent = m_pEnv->m_agents[m_pEnv->m_activeAgent];
        std::for_each(
            pos.begin()
          , pos.end()
          , [&](CEnvAction::CPossibleActions::const_reference r) 
            {
                double actionValue = activeAgent->m_ptrFunc->GetValue(
                    translate(m_pEnv->m_ptrState, r, m_pEnv->m_activeAgent)
                  );
                variants.insert(std::make_pair(actionValue, r));
            }
          );

        CValueActionMap::const_iterator iter = m_policy.Select(variants);
        m_performedValue = iter->first;
        m_ptrPerformedAction = iter->second;
        CValueActionMap::const_iterator greedyIter = --variants.end();
        m_greedyValue = greedyIter->first;
        m_ptrGreedyAction = greedyIter->second;

        // GET NEXT STATE AND REWARDS
        m_pEnv->m_ptrPrevState = m_pEnv->m_ptrState;
        m_pEnv->m_ptrState = m_pEnv->getNextState(m_pEnv->m_ptrState, m_ptrPerformedAction);
        CVectorDbl rewards(m_pEnv->m_agents.size(), rll_type(0.0));
        m_pEnv->observeRewards(rewards);

        // UPDATE VALUE FUNCTION
        if (finished = m_pEnv->isEpisodeFinished()) {
            performAgentUpdate(rewards);
            // If episode has finished
            for (std::vector<CAgentPtr>::size_type i=0; i < m_pEnv->m_agents.size(); ++i) {
                double reward = m_pEnv->m_agents[i]->m_reward;
                m_pEnv->m_agents[i]->m_reward = 0.0;
                if (m_pEnv->m_agents[i]->m_ptrPrevState != NULL) {
                    updateValueFunction(static_cast<int>(i), reward, true);
                }
            }
        } else {
            performAgentUpdate(rewards);
        }

        // Change active agent
        m_pEnv->selectNextAgent();

        ++m_pEnv->m_step;
    } while (!finished);
}

CVectorRlltPtr translate(
    const CStatePtr& i_ptrState
  , const CActionPtr& i_ptrAction
  , int i_agent) 
{
    CVectorRlltPtr ptrRet = i_ptrState->GetData();

    if (i_ptrAction) {
        ptrRet->push_back(i_ptrAction->GetData());
    }
    ptrRet->push_back(i_agent);
    return ptrRet;    
}

}}
