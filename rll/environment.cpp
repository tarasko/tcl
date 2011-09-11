#include "environment.hpp"
#include "method.hpp"

namespace tcl { namespace rll {

std::vector<CAgentPtr>& CEnvBase::agents()
{
    return m_agents;
}

const std::vector<CAgentPtr>& CEnvBase::agents() const
{
    return m_agents;
}

unsigned int CEnvBase::episode() const
{
    return m_method->episode();
}

unsigned int CEnvBase::step() const
{
    return m_method->step();
}

}}