#include "method.hpp"
#include "environment.hpp"
#include "agent.hpp"
#include "state.hpp"
#include "action.hpp"

#include "detail/logger.hpp"
#include "detail/utils.hpp"

#include <functional>

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

        m_variants.resize(nextStates.size());

        std::transform(
            nextStates.begin()
          , nextStates.end()
          , m_variants.begin()
          , [&](const CStatePtr& state) -> std::pair<double, CStatePtr>
            {
                double stateValue = activeAgent->getValue(
                    detail::translate(state, CActionPtr(), activeAgentIdx)
                  );

                return std::make_pair(stateValue, state);
            }
          );

        // Sort it
        std::sort(
            m_variants.begin()
          , m_variants.end()
          , [](CValueStateMap::const_reference r1, CValueStateMap::const_reference r2) -> bool
            {
                return r1.first < r2.first;
            }
          );

        // Select action and therefore next state according policy 
        CValueStateMap::const_reference policySelection = m_policy.select(m_variants);
        CValueStateMap::const_reference greedySelection = m_variants.back();

        // Get reward for agent 
        if (pEnv->setNextStateAssignRewards(policySelection.second))
        {
            // Update value function
            updateValueFunctionImpl(
                activeAgent
              , activeAgentIdx
              , policySelection
              , activeAgent->releaseReward()
              );

            // Remember new state for active agent
            activeAgent->setLastStateWhenWasActive(policySelection.second);
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

    // Update value function for last state for each agent according to terminal rewards
    auto terminalState = std::make_pair(0.0, CStatePtr());
    for(size_t agentIdx = 0; agentIdx < terminalRewards.size(); ++agentIdx) 
    {
        updateValueFunctionImpl(
            pEnv->agents()[agentIdx]
          , agentIdx
          , terminalState
          , terminalRewards[agentIdx]
          );
    }
}

CActionMethod::CActionMethod(CEnvAction* pEnv, const CConfigPtr& ptrConfig) 
    : CMethodBase(pEnv, ptrConfig)
{
}

void CActionMethod::runEpisode() 
{
    CEnvAction* pEnv = static_cast<CEnvAction*>(m_pEnv);

    pEnv->initEpisode();
    int activeAgentIdx;       // Current active agent index
    bool cont = true;
    
    // 1. Get possible actions for active agent.
    // 2. Select action according policy.
    // 3. Update value function. We can do this step cause we know Q(t-1) and Q(t) and last reward.
    // In case of first move we don`t do update.
    // Probably here must be offline and online updates.
    // 4. Make last selected action, agents can recieve rewards on this step. 
    // All this rewards are remembered until we will know next state-action pair.
    // On this step environment also can tell that we reached terminal state.
    // If we are in terminal state go to step 5. Else go to step 1.
    // 5. Iterate over all agents, pretend that next state-action pair will have value function 0.0
    // Update according to last reward.

    for (m_step = 0; cont; ++m_step)
    {
        // Get active agent
        activeAgentIdx = m_pEnv->activeAgent();
        CAgentPtr activeAgent = m_pEnv->agents()[activeAgentIdx];

        // 1. Get possible actions for active agent.
        auto possibleActions = pEnv->getPossibleActions();
        if (possibleActions.empty()) {
            throw CRLException("At least must be one possible next action");
        }

        // 2. Select action according policy.
        CStatePtr currentState = pEnv->currentState();
        typedef std::vector<std::pair<double, CActionPtr> > CValueActionMap;

        // TODO: This can be done on one time preallocated memory
        m_variants.resize(possibleActions.size());
        std::transform(
            possibleActions.begin()
          , possibleActions.end()
          , m_variants.begin()
          , [&](const CActionPtr& a) -> std::pair<double, CActionPtr>
            {
                double actionValue = activeAgent->getValue(
                    detail::translate(currentState, a, activeAgentIdx)
                  );

                return std::make_pair(actionValue, a);
            }
          );

        // Sort it
        std::sort(
            m_variants.begin()
          , m_variants.end()
          , [](CValueActionMap::const_reference r1, CValueActionMap::const_reference r2) -> bool
            {
                return r1.first < r2.first;
            }
          );

        CValueActionMap::const_reference policySelection = m_policy.select(m_variants);
        CValueActionMap::const_reference greedySelection = m_variants.back();

        // 3. Update value function. We can do this step cause we know Q(t-1) and Q(t) and last reward.
        // In case of first move we don`t do update.
        // Probably here must be offline and online updates.
        if (activeAgent->lastStateWhenWasActive()) {
            updateValueFunctionImpl(
                activeAgent
              , activeAgentIdx
              , currentState
              , policySelection
              , greedySelection
              , activeAgent->releaseReward()
              );
        }

        activeAgent->setLastStateWhenWasActive(currentState);
        activeAgent->setLastActionWhenWasActive(policySelection.second);

        // 4. Make last selected action, agents can recieve rewards on this step. 
        // All this rewards are remembered until we will know next state-action pair.
        // On this step environment also can tell that we reached terminal state.
        // If we are in terminal state go to step 5. Else go to step 1.
        cont = pEnv->doActionAssignRewards(policySelection.second);
    }

    // Get terminal rewards for all agents
    CVectorDbl terminalRewards(m_pEnv->agents().size());
    std::transform(
        m_pEnv->agents().begin()
      , m_pEnv->agents().end()
      , terminalRewards.begin()
      , std::mem_fn(&CAgent::releaseReward)
      );

    // 5. Iterate over all agents, pretend that next state-action pair will have value function 0.0
    // Update according to last reward.
    // Update value function for last state for each agent according to terminal rewards
    auto terminalAction = std::make_pair(0.0, CActionPtr());
    for(size_t agentIdx = 0; agentIdx < terminalRewards.size(); ++agentIdx) 
    {
        updateValueFunctionImpl(
            pEnv->agents()[agentIdx]
          , agentIdx
          , CStatePtr()
          , terminalAction
          , terminalAction
          , terminalRewards[agentIdx]
          );
    }
}

}}
