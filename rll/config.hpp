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
        m_accumulating(false),
        m_policy(EPSILON_GREEDY),
        m_epsilon(0.05),
        m_hidden(10),
        m_vfMin(-1.0),
        m_vfMax(1.0),
        m_etEpsilon(0.001),
        m_enableLog(false)
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
};

}}
