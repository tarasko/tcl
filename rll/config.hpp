#pragma once

#include "rll_fwd.hpp"

#include <memory>

namespace tcl { namespace rll {

struct CConfig 
{
    enum POLICY_TYPE
    {
        GREEDY,
        EPSILON_GREEDY,
        SOFTMAX
    };

    CConfig() : 
        m_alpha(0.05),
        m_lambda(0.5),
        m_gamma(1.0),
        m_accumulating(true),
        m_policy(EPSILON_GREEDY),
        m_epsilon(0.05),
        m_hidden(10),
        m_vfMin(-1.0),
        m_vfMax(1.0),
        m_etEpsilon(0.001),
        m_enableLog(false),
        m_rewardTerminal(true) 
    {
    }

    /** @brief Alpha - stepsize parameter for RL methods */
    double m_alpha;
    /** @brief Lambda used in eligibility traces */
    double m_lambda;
    /** @brief Gamma - discount rate for return calculating */
    double m_gamma;
    /** @brief Should use accumulationg trace or replacing */
    bool m_accumulating;
    /** @brief Policy type */
    POLICY_TYPE m_policy;
    /** @brief Epsilon constatn for epsilon-greedy policy */
    double m_epsilon;

    /** @name Neuronal network settings */
    //@{
    /** @brief Number of neurons in hidden layer */
    unsigned int m_hidden;
    /** @brief Minimal value for value function */
    double m_vfMin;
    /** @brief Maximal value for value function */
    double m_vfMax;
    //@}

    /** @brief Epsilon value for eligibility traces.
    If trace value for state is less then this it would be eliminated */
    double m_etEpsilon;
    /** @brief Enable or disable log */
    bool m_enableLog;
    /** @brief Reward terminal state with previous occured reward?
        When the episode is over we know significant result not for terminal state
        but only for state before terminal. So the terminal state always has value 
        0. And it`s actually correct because this is state method niether 
        state-action. So when we got state before last we cannot make next move 
        based only value prediction. For example for wining state we have always 
        zero prediction. This is bad. The worst thing is that there is another 
        special case. Suppose we use some time stamp as condition of episode end (not
        some special terminal state). Here last state can be equal to every state 
        that occurs before.
        Solution that I have is partial. Make prediction for every terminal state 
        equal to reward we got before this state. This is optionally and can be 
        changed from Config object. Set property 'm_rewardTerminal' to obtain such 
        behavior. This will work well only if there are set of terminal states and if 
        agent got to any of that states the episode will immidiatly finish. */
    bool m_rewardTerminal;
};

}}
