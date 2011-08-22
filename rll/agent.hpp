#pragma once

#include "types.hpp"
#include "state.hpp"
#include "value_function.hpp"

#include <list>
#include <memory>

namespace tcl { namespace rll {

/** @brief Helper struct for agents.
    Contain various helpful information about agent.
    Agent internal state differs from internal state stored in Method.
    Additionaly it contains agent index on the bottom */
class CAgent 
{
public:
    CAgent(CValueFunctionPtr i_ptrFunc);

    typedef std::vector<CVectorRlltPtr> CPreviousStates;
    typedef std::list<std::pair<CVectorRlltPtr, double> > CTracesMap;

    /// @brief Add or update state value to eligibility trace
    /// 
    /// If the state already has been visited and we found it in traces map then 
    /// update trace value depend on update strategy: accumulating or replacing.
    /// If state occured for first time we add it to traces map and bind 1.0 value.
    /// Also perform traces cleanup. Remove every trace that has value less then 
    /// i_etEpsilon.
    /// 
    /// @param i_ptrState - state that we occured on this episode step.
    /// @param i_accum - if true then use accumulating traces if false then 
    /// replacing. 
    /// @param i_etEpsilon - if trace value is less than this, remove it from traces
    /// map.
    void UpdateTrace(
        const CVectorRlltPtr& i_ptrState
      , bool i_accum
      , double  i_etEpsilon
      );

    /// @brief Clean agent episode temporaries
    void Clean();

    double m_reward;              //!< Accumulated reward after few steps of waiting to become active
    CVectorRlltPtr m_ptrPrevState; //!< Last agent state when it was active
    CTracesMap m_traces;          //!< Agent eligibility traces
    CValueFunctionPtr m_ptrFunc;  //!< Value function for agent
};

}}
