#pragma once

#include "value_function.hpp"
#include "config.hpp"

namespace tcl { namespace rll {

/// @brief Implements value fucntion based on neuronal network.
class CNeuronalNetwork : public CValueFunction 
{
public:
    CNeuronalNetwork(const CConfigPtr& i_ptrConfig);

    /// @brief Return value for internal representation of state.
    virtual double getValue(const CVectorRlltPtr& i_ptrState);
    /// @brief Correct value function according update map
    virtual void update(const CUpdateList& i_list);

protected:
    /// @brief Wrap neuronal network created by FANN library
    class CFannWrapper;

    CFannWrapper* m_pFann;     //!< Use lazy init for it
    CConfigPtr    m_ptrConfig; //!< Configuration object
};

}}
