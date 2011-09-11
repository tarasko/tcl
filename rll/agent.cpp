#include "agent.hpp"
#include "value_function.hpp"

#include <cassert>
#include <algorithm>

using std::make_pair;

namespace tcl { namespace rll {

CAgent::CAgent(const CValueFunctionPtr& i_ptrFunc) 
    : m_ptrFunc(i_ptrFunc)
    , m_reward(0.0)
{
    assert(m_ptrFunc);
}

double CAgent::getValue(const CVectorRlltPtr& i_ptrState)
{
    return m_ptrFunc->getValue(i_ptrState);
}

void CAgent::update(const CUpdateList& i_list)
{
	m_ptrFunc->update(i_list);
}

void CAgent::addReward(double reward)
{
    m_reward += reward;
}

double CAgent::releaseReward()
{
    double tmp = m_reward;
    m_reward = 0.0;
    return tmp;
}

}}
