#include "environment.hpp"
#include "method.hpp"

namespace tcl { namespace rll {

std::vector<agent_sp>& env_base::agents()
{
    return agents_;
}

const std::vector<agent_sp>& env_base::agents() const
{
    return agents_;
}

unsigned int env_base::episode() const
{
    return method_->episode();
}

unsigned int env_base::step() const
{
    return method_->step();
}

}}