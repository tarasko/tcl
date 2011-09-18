#pragma once

#include "value_function.hpp"
#include "config.hpp"

namespace tcl { namespace rll {

/// @brief Implements value fucntion based on neuronal network.
class vf_neuronal_network : public value_function 
{
public:
    vf_neuronal_network(const CConfigPtr& config);

    /// @brief Return value for internal representation of state.
    virtual double get_value(const vector_rllt_csp& st);
    /// @brief Correct value function according update map
    virtual void update(const update_list& lst);

protected:
    /// @brief Wrap neuronal network created by FANN library
    class fann_wrapper;

    fann_wrapper* fann_wrapper_;  //!< Use lazy init for it
    CConfigPtr    config_;        //!< Configuration object
};

}}
