#include "method_base.hpp"
#include "../environment.hpp"
#include "../agent.hpp"

#include <functional>
#include <algorithm>

namespace tcl { namespace rll { namespace detail {

void method_base::run(unsigned int episodes)
{
    env_->method_ = this;

    for (episode_ = 0; episode_ < episodes; ++episode_) try
    {
        step_ = 0;

        // Reset previous state for all agents
        std::for_each(
            env_->agents().begin()
          , env_->agents().end()
          , [](agent_sp& agent) -> void
            {
                agent->set_prev_state(vector_rllt_csp());
            }
          );

        run_episode_impl();
    }
    catch(...)
    {
        env_->method_ = 0;
        throw;
    }

    env_->method_ = 0;
}

}}}
