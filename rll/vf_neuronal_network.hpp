#pragma once

#include "value_function.hpp"

struct fann;

namespace tcl { namespace rll {

/// @brief Implements value fucntion based on neuronal network.
class vf_neuronal_network : public value_function 
{
public:
    vf_neuronal_network(
        unsigned int hidden
      , double min_val = -1.0
      , double max_val = 1.0
      );
    ~vf_neuronal_network();

    /// @brief Return value for internal representation of state.
    virtual double get_value(const vector_rllt_csp& st);
    /// @brief Correct value function according update map
    virtual void update(const update_list& lst);

private:
    fann* create_nn(size_t input_size);

    /// @brief Use center and scale to make outputs for training set.
    double scale_in(double val);

    /// @brief Use center and scale to make val func from network output.
    double scale_out(double val);

    fann* fann_;
    unsigned int hidden_;
    double min_;
    double max_;
    double center_; //!< Defines the center beetwen min and max values
    double scale_;  //!< Defines the scale rate for output values

};

}}
