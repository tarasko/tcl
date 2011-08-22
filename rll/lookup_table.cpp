#include "lookup_table.hpp"

using std::make_pair;

namespace tcl { namespace rll {

double CLookupTable::GetValue(const CVectorRlltPtr& i_ptrState) 
{
    CValueMap::iterator i = m_values.find(i_ptrState);
    if (i != m_values.end()) {
        return i->second;
    } else {
        m_values.insert(make_pair(i_ptrState, m_init));
        return m_init;
    }
}

void CLookupTable::Update(const CUpdateList& i_list) 
{
    for (CUpdateList::const_iterator i = i_list.begin(); i != i_list.end(); ++i) 
    {
        // Find and if not exist create record for state
        CValueMap::iterator record = m_values.find(i->first);
        if (m_values.end() == record) {
            record = m_values.insert(make_pair(i->first, m_init)).first;
        }

        record->second = i->second;
    }
}

}}
