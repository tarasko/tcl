#pragma once

#include "../rll_fwd.hpp"
#include "../state.hpp"
#include "../config.hpp"

namespace tcl { namespace rll { namespace detail {

/// @brief Base class for RL methods.
class method_base 
{
public:
    method_base(env_base* env, const config& config);
    virtual ~method_base();

    /// @brief Run training loop.
    /// @param episodes - Number of episodes to train
    void run(unsigned int episodes);

    unsigned int episode() const;
    unsigned int step() const;

protected:
    /// @brief Run single episode.
    virtual void run_episode_impl() = 0;  

    /// @brief Update value function for specific agent with new reward
    /// @param active_agent - current active agent
    /// @param active_agent - active agent index
    /// @param new_state_value - estimated value for new state returned by value function
    /// When next state is terminal it should be 0.0
    /// @param reward - reward that agent got by selecting @c new state
    virtual void update_value_function_impl(
        const agent_sp& active_agent
      , int active_agent_idx
      , double policy_selection_value
      , double greedy_selection_value
      , double reward
      ) = 0;

    config          config_;     //!< Config information
    env_base*       env_;        //!< Environment

    unsigned int    episode_;    //!< Current episode
    unsigned int    step_;       //!< Current step in episode
};

inline method_base::method_base(env_base* env, const config& config)
    : config_(config)
    , env_(env)
    , episode_(0)
    , step_(0)
{
    assert(env);
}

inline method_base::~method_base()
{
}

inline unsigned int method_base::episode() const
{
    return episode_;
}

inline unsigned int method_base::step() const
{
    return step_;
}

}}}
