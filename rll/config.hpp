#pragma once

#include "rll_fwd.hpp"

#include <memory>

namespace tcl { namespace rll {

struct CConfig 
{
    CConfig() : 
        m_alpha(0.05),
        m_lambda(0.5),
        m_gamma(1.0),
        m_accumulating(false),
        m_etEpsilon(0.001)
    {
    }

    double m_alpha;  //!< Alpha - stepsize parameter for RL methods.
    double m_lambda; //!< Lambda used in eligibility traces.
    double m_gamma;  //!< Gamma - discount rate for return calculating.
    bool m_accumulating;  //!< Should use accumulationg trace or replacing.

    /// @brief Epsilon value for eligibility traces.
    /// If trace value for state is less then this it would be eliminated.
    double m_etEpsilon;
};

}}
