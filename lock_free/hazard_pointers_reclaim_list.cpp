#include "hazard_pointers_reclaim_list.hpp"

#include <boost/thread/tss.hpp>

namespace tcl { namespace lock_free {

boost::thread_specific_ptr<hazard_pointers_reclaim_list> g_list;

hazard_pointers_reclaim_list& get_reclaim_list()
{
	if (!g_list.get())
		g_list.reset(new hazard_pointers_reclaim_list);
	
	return *g_list;
}

}}
