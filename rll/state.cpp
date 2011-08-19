#include "state.hpp"

#include <algorithm>

namespace tcl { namespace rll {

void CState::RegisterVariable(const std::string& i_name) 
{
    if (m_vars.find(i_name) != m_vars.end()) {
        throw CStateError("Variable with such name already exists\n");
    } else {
        m_vars[i_name] = 0;
    }
}

void CState::RevokeVariable(const std::string& i_name) 
{
    CVarMap::iterator i = m_vars.find(i_name);
    if (i == m_vars.end()) {
        throw CStateError("Cannot find variable with such name\n");
    } else {
        m_vars.erase(i);
    }
}

void CState::SetValue(const std::string& i_name, double i_val) 
{
    CVarMap::iterator i = m_vars.find(i_name);
    if (i == m_vars.end()) {
        throw CStateError("Cannot find variable");
    } else {
        i->second = i_val;
    }
}

double CState::GetValue(const std::string& i_name) 
{
    CVarMap::iterator i = m_vars.find(i_name);
    if (i == m_vars.end()) {
        throw CStateError("Cannot find variable");
    } else {
        return i->second;
    }
}

CVectorDblPtr CState::GetData() const 
{
    CVectorDblPtr result = std::make_shared<CVectorDbl>(m_vars.size());

    std::transform(
        m_vars.begin()
      , m_vars.end()
      , result->begin()
      , [](CVarMap::const_reference r) 
        {
            return r.second;
        }
      );

    return result;
}

CStatePtr CState::Clone() const 
{
    return std::make_shared<CState>(*this);
}

bool CState::IsEqual(const CStatePtr& i_ptrState) const
{
    return std::equal(
        m_vars.begin()
      , m_vars.end()
      , i_ptrState->m_vars.begin()
      , [](CVarMap::const_reference r1, CVarMap::const_reference r2)
        {
            return r1.second == r2.second;
        }
      );
}

}}
