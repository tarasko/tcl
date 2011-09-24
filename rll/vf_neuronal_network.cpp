#include "vf_neuronal_network.hpp"

#include "detail/fann/doublefann.h"

#include <iterator>
#include <algorithm>
#include <cassert>

using namespace std;

namespace tcl { namespace rll {

/// @todo Do not perform copy of elements for rll double.

vf_neuronal_network::vf_neuronal_network(
    unsigned int hidden
  , double min_val
  , double max_val
  )
  : fann_(0)
  , hidden_(hidden)
  , min_(min_val)
  , max_(max_val)
{
    center_ = (max_ + min_) / 2;
    scale_ = 1 / (center_ - min_);
}

vf_neuronal_network::~vf_neuronal_network()
{
    fann_destroy(fann_);
}

double vf_neuronal_network::get_value(const vector_rllt_csp& st) 
{
    assert(!st.empty());

    if (!fann_)
        fann_ = create_nn(st->size());

    vector<double> in(st->begin(), st->end());

    return static_cast<double>(scale_out(*fann_run(fann_, &in[0])));
}

void vf_neuronal_network::update(const update_list& ul) 
{
    assert(!ul.empty());

    if (!fann_)
        fann_ = create_nn(ul.front().first->size());

    // Transform update_list to appropriate representation
    typedef vector<pair<vector<double>, double> > internal_update_list;
    internal_update_list intData(ul.size());

    for (update_list::size_type i=0; i<ul.size(); ++i) 
    {
        /// @todo Resize or reserve + emplace
        // Copy first member
        std::copy(
            ul[i].first->begin()
          , ul[i].first->end()
          , back_inserter(intData[i].first)
          );
        // Copy second member and make scalling
        intData[i].second = scale_in(static_cast<double>(ul[i].second));
    }

    // Fill train data structure
    fann_train_data td;
    td.num_data = static_cast<int>(intData.size());
    td.num_input = static_cast<int>(intData[0].first.size());
    td.num_output = 1;
    vector<double*> input(intData.size());
    vector<double*> output(intData.size());
    td.input = &input[0];
    td.output = &output[0];

    // And at last set actual pointers
    for (update_list::size_type i=0; i<ul.size(); ++i) 
    {
        input[i] = &(intData[i].first[0]);
        output[i] = &(intData[i].second);
    }

    /// @todo Probably we should return it somehow.
    float mse = fann_train_epoch(fann_, &td);
}

fann* vf_neuronal_network::create_nn(size_t input_size)
{
    fann_ = fann_create_standard(3, input_size, hidden_, 1);
    if (!fann_)
        throw std::bad_alloc("fann_create_standart: unable to create network");

    fann_set_training_algorithm(fann_, FANN_TRAIN_INCREMENTAL);
    fann_set_activation_function_output(fann_, FANN_SIGMOID_SYMMETRIC);

    return fann_;
}

double vf_neuronal_network::scale_in(double val)
{
    val = max(val, min_);
    val = min(val, max_);
    return (val - center_) * scale_;
}

double vf_neuronal_network::scale_out(double val)
{
    return val / scale_ + center_;
}

}}
