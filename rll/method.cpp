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

CMethodBase::CMethodBase(CEnvBase* pEnv, const CConfigPtr& ptrConfig) 
    : m_ptrConfig(ptrConfig)
    , m_policy(ptrConfig) 
    , m_pEnv(pEnv)
    , m_episode(0)
    , m_step(0)
{
}

CMethodBase::~CMethodBase()
{
}

void CMethodBase::run(unsigned int episodes) 
{
    if (!m_ptrConfig) 
        throw CRLException("m_ptrConfig member cannot be null");

    g_log.Enable(m_ptrConfig->m_enableLog);
    m_pEnv->m_method = this;

    struct OnScopeExit
    {
        OnScopeExit(const CMethodBase*& ref) : m_ref(ref) 
        {
        }
        ~OnScopeExit()
        {
            m_ref = 0;
            g_log.Enable(false);
        }

    private:
        const CMethodBase*& m_ref;
    }
    onScopeExit(m_pEnv->m_method);

    for (m_episode = 0; m_episode < episodes; ++m_episode)
    {
        m_step = 0;

        // Reset all agents
        std::for_each(
            m_pEnv->agents().begin()
          , m_pEnv->agents().end()
          , [](CAgentPtr& agent) -> void
            { 
                agent->setLastStateWhenWasActive(CStatePtr());
                agent->setLastActionWhenWasActive(CActionPtr());
            }
          );

        runEpisode();    
    }
}


unsigned int CMethodBase::episode() const
{
    return m_episode;
}

unsigned int CMethodBase::step() const
{
    return m_step;
}

CStateMethod::CStateMethod(CEnvState* pEnv, const CConfigPtr& ptrConfig) 
    : CMethodBase(pEnv, ptrConfig)
{
}

void CStateMethod::runEpisode() 
{
    CEnvState* pEnv = static_cast<CEnvState*>(m_pEnv);

    pEnv->initEpisode();
    int activeAgentIdx;     // Current active agent index
    
    // Repeat for each step in episode
    // Break when setNextStateAssignRewards return false
    for(m_step = 0;; ++m_step)
    {
        // Get active agent on this step
        activeAgentIdx = pEnv->activeAgent();
        CAgentPtr activeAgent = pEnv->agents()[activeAgentIdx];

        if (!activeAgent->lastStateWhenWasActive()) 
            activeAgent->setLastStateWhenWasActive(pEnv->currentState());

        // Get possible next states for current active agent
        auto nextStates = pEnv->getPossibleNextStates();
        if (nextStates.empty()) {
            throw CRLException("at least must be one possible next state");
        }

        // Get value for every possible state
        typedef std::multimap<double, CStatePtr> CValueStateMap;
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
        if (pEnv->setNextStateAssignRewards(iter->second))
        {
            // Update value function
            updateValueFunctionImpl(
                activeAgent
              , activeAgentIdx
              , iter->second
              , activeAgent->releaseReward()
              );

            // Remember new state for active agent
            activeAgent->setLastStateWhenWasActive(iter->second);
        }
        else
            break; // finish loop after we got to terminal state
    }

    // Get terminal rewards for all agents
    CVectorDbl terminalRewards(m_pEnv->agents().size());
    std::transform(
        m_pEnv->agents().begin()
      , m_pEnv->agents().end()
      , terminalRewards.begin()
      , std::mem_fn(&CAgent::releaseReward)
      );

    // Update value for active agent
    // We assume here that terminal state value function is 0.
    updateValueFunctionImpl(
        pEnv->agents()[activeAgentIdx]
      , activeAgentIdx
      , CStatePtr()
      , terminalRewards[activeAgentIdx]
      );

    //// Update value function for last state for each agent according to terminal rewards
    //for(size_t agentIdx = 0; agentIdx < terminalRewards.size(); ++agentIdx) 
    //{
    //    updateValueFunctionImpl(
    //        pEnv->agents()[agentIdx]
    //      , agentIdx
    //      , CStatePtr()
    //      , terminalRewards[agentIdx]
    //      );
    //}
}

CActionMethod::CActionMethod(CEnvAction* pEnv, const CConfigPtr& ptrConfig) 
    : CMethodBase(pEnv, ptrConfig)
{
}

void CActionMethod::runEpisode() 
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
    //     typedef std::multimap<double, CActionPtr> CValueActionMap;
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
