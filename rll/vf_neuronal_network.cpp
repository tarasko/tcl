#include "vf_neuronal_network.hpp"

#include "detail/fann/doublefann.h"

#include <iterator>
#include <algorithm>
#include <cassert>

using namespace std;

namespace tcl { namespace rll {

/// @todo Investigate errors with FANN_LINEAR in output layer.
/// @todo Do not make copy of data in case of int
class vf_neuronal_network::fann_wrapper 
{
public:
    fann_wrapper(int i_input, CConfigPtr config) 
    {
        fann_wrapper_ = fann_create_standard(3, i_input, config->m_hidden, 1);
        assert(fann_wrapper_);
        fann_set_training_algorithm(fann_wrapper_, FANN_TRAIN_INCREMENTAL);
        fann_set_activation_function_output(fann_wrapper_, FANN_LINEAR);
        // Calculate center and scale
        m_min = config->m_vfMin;
        m_max = config->m_vfMax;
        m_center = (m_max + m_min) / 2;
        m_scale = 1 / (m_center - m_min);
    }

    ~fann_wrapper() 
    {
        fann_destroy(fann_wrapper_);
    }

    /// @brief Incremental training on examples.
    void update(const update_list& lst) 
    {
        if (lst.empty()) {
            return;
        }

        // Transform UpdataList to appropriate representation
        typedef vector<pair<vector<fann_type>, fann_type> > CInternalList;
        CInternalList intData(lst.size());
        for (update_list::size_type i=0; i<lst.size(); ++i) {
            // Copy first member
            std::copy(
                lst[i].first->begin()
              , lst[i].first->end()
              , back_inserter(intData[i].first)
              );
            // Copy second member and make scalling
            intData[i].second = scaleIn(static_cast<fann_type>(lst[i].second));
        }

        // Fill train data structure
        fann_train_data td;
        td.num_data = static_cast<int>(intData.size());
        td.num_input = static_cast<int>(intData[0].first.size());
        td.num_output = 1;
        vector<fann_type*> input(intData.size());
        vector<fann_type*> output(intData.size());
        td.input = &input[0];
        td.output = &output[0];

        // And at last set actual pointers
        for (update_list::size_type i=0; i<lst.size(); ++i) {
            input[i] = &(intData[i].first[0]);
            output[i] = &(intData[i].second);
        }

        float mse = fann_train_epoch(fann_wrapper_, &td);
    }

    /// @brief Return value for internal representation of state.
    virtual double get_value(const vector_rllt_csp& st) {
        if (st->empty()) {
            return 0.0;
        }
        vector<fann_type> in(st->begin(), st->end());
        return static_cast<double>(scaleOut(*fann_run(fann_wrapper_, &in[0])));
    }

private:
    /// @brief Use center and scale to make outputs for training set.
    fann_type scaleIn(fann_type i_val) 
    {
        return i_val;
        //i_val = max(i_val, m_min);
        //i_val = min(i_val, m_max);
        //return (i_val - m_center) * m_scale;
    }

    /// @brief Use center and scale to make val func from network output.
    fann_type scaleOut(fann_type i_val) 
    {
        return i_val;
        //return i_val / m_scale + m_center;
    }

    fann*  fann_wrapper_;  //!< FANN neuronal network
    double m_center; //!< Defines the center beetwen min and max values
    double m_scale;  //!< Defines the scale rate for output values
    double m_min;    //!< Defines minimum possible output value
    double m_max;    //!< Defines maximum possible output value
};

vf_neuronal_network::vf_neuronal_network(const CConfigPtr& config) 
    : fann_wrapper_(NULL)
    , config_(config) 
{
}

double vf_neuronal_network::get_value(const vector_rllt_csp& st) {
    if (st->empty()) {
        return 0.0;
    }

    if (!fann_wrapper_) {
        fann_wrapper_ = new fann_wrapper((int)st->size(), config_);
    }

    return fann_wrapper_->get_value(st);
}

void vf_neuronal_network::update(const update_list& i_map) {
    if (i_map.empty()) {
        return;
    }

    if (!fann_wrapper_) {
        fann_wrapper_ = new fann_wrapper((int)i_map[0].first->size(), config_);
    }

    return fann_wrapper_->update(i_map);
}

}}
