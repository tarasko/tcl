#include "vf_lookup_table.hpp"

using namespace std;

namespace tcl { namespace rll {

double vf_lookup_table::get_value(const vector_rllt_csp& st) 
{
    value_map::iterator i = values_.find(st);
    if (i != values_.end()) 
        return i->second;
    else 
    {
        values_.insert(make_pair(st, init_));
        return init_;
    }
}

void vf_lookup_table::update(const update_list& lst) 
{
    std::for_each(lst.begin(), lst.end(), [&](update_list::const_reference r) -> void 
    {
        values_[r.first] = r.second;
    });
}

}}

