#include "environment.hpp"

#include <algorithm>

namespace tcl { namespace rll {

void CEnvBase::initEpisode()
{
    ++m_episode;
	m_step = 0;
    std::for_each(
        m_agents.begin()
      , m_agents.end()
      , [](CAgentPtr& agent) -> void
        { 
            agent->setLastStateWhenWasActive(CStatePtr());
            agent->setLastActionWhenWasActive(CActionPtr());
        }
      );

    initEpisodeImpl();
    m_agents[activeAgentImpl()]->setLastStateWhenWasActive(currentStateImpl());
}

bool CEnvBase::nextStep()
{
    bool cont = nextStepImpl();
    if (cont) 
    {
        if (!m_agents[activeAgentImpl()]->lastStateWhenWasActive()) 
            m_agents[activeAgentImpl()]->setLastStateWhenWasActive(currentStateImpl());

        ++m_step;
    }
    return cont;
}

}}