#include "neuronal_network.hpp"

#include "detail/logger.hpp"
#include "detail/fann/doublefann.h"

#include <iterator>
#include <algorithm>
#include <cassert>

using namespace std;

namespace tcl { namespace rll {

/// @todo Investigate errors with FANN_LINEAR in output layer.
/// @todo Do not make copy of data in case of int
class CNeuronalNetwork::CFannWrapper 
{
public:
    CFannWrapper(int i_input, CConfigPtr i_ptrConfig) 
    {
        m_pFann = fann_create_standard(3, i_input, i_ptrConfig->m_hidden, 1);
        assert(m_pFann);
        fann_set_training_algorithm(m_pFann, FANN_TRAIN_INCREMENTAL);
        fann_set_activation_function_output(m_pFann, FANN_LINEAR);
        // Calculate center and scale
        m_min = i_ptrConfig->m_vfMin;
        m_max = i_ptrConfig->m_vfMax;
        m_center = (m_max + m_min) / 2;
        m_scale = 1 / (m_center - m_min);
    }

    ~CFannWrapper() 
    {
        fann_destroy(m_pFann);
    }

    /// @brief Incremental training on examples.
    void update(const CUpdateList& i_list) 
    {
        if (i_list.empty()) {
            return;
        }

        // Transform UpdataList to appropriate representation
        typedef vector<pair<vector<fann_type>, fann_type> > CInternalList;
        CInternalList intData(i_list.size());
        for (CUpdateList::size_type i=0; i<i_list.size(); ++i) {
            // Copy first member
            std::copy(
                i_list[i].first->begin()
              , i_list[i].first->end()
              , back_inserter(intData[i].first)
              );
            // Copy second member and make scalling
            intData[i].second = scaleIn(static_cast<fann_type>(i_list[i].second));
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
        for (CUpdateList::size_type i=0; i<i_list.size(); ++i) {
            input[i] = &(intData[i].first[0]);
            output[i] = &(intData[i].second);
        }

        g_log.Print(td);

        float mse = fann_train_epoch(m_pFann, &td);
    }

    /// @brief Return value for internal representation of state.
    virtual double getValue(const CVectorRlltPtr& i_ptrState) {
        if (i_ptrState->empty()) {
            return 0.0;
        }
        vector<fann_type> in(i_ptrState->begin(), i_ptrState->end());
        return static_cast<double>(scaleOut(*fann_run(m_pFann, &in[0])));
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

    fann*  m_pFann;  //!< FANN neuronal network
    double m_center; //!< Defines the center beetwen min and max values
    double m_scale;  //!< Defines the scale rate for output values
    double m_min;    //!< Defines minimum possible output value
    double m_max;    //!< Defines maximum possible output value
};

CNeuronalNetwork::CNeuronalNetwork(const CConfigPtr& i_ptrConfig) 
    : m_pFann(NULL)
    , m_ptrConfig(i_ptrConfig) 
{
}

double CNeuronalNetwork::getValue(const CVectorRlltPtr& i_ptrState) {
    if (i_ptrState->empty()) {
        return 0.0;
    }

    if (!m_pFann) {
        m_pFann = new CFannWrapper((int)i_ptrState->size(), m_ptrConfig);
    }

    return m_pFann->getValue(i_ptrState);
}

void CNeuronalNetwork::update(const CUpdateList& i_map) {
    if (i_map.empty()) {
        return;
    }

    if (!m_pFann) {
        m_pFann = new CFannWrapper((int)i_map[0].first->size(), m_ptrConfig);
    }

    return m_pFann->update(i_map);
}

}}
