#include "../lf_stack_hp.hpp"
#include "../lb_stack.hpp"
#include "../lf_stack_refcnt.hpp"
#include "../lf_spsc_queue.hpp"
#include "../lf_mpmc_queue.hpp"
#include "../lb_queue.hpp"
#include "../lb_fg_queue.hpp"
#include "../../allocators/fixed_allocator.hpp"

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/atomic.hpp>
#include <boost/chrono/chrono.hpp>
#include <boost/chrono/chrono_io.hpp>

#include <iostream>

typedef boost::chrono::steady_clock clock_type;
const int NUM_ATTEMPTS = 10000;

boost::mutex g_log_guard;

using namespace std;
using namespace tcl::containers;

//void pop_proc()
//{
//	for(;;)
//		g_stack.pop();
//}
//
//void push_proc()
//{
//	for(int i = 0;; ++i)
//		g_stack.push(i);
//}

template<typename Container>
void measured_pop_proc(Container& c, boost::barrier& b, const char* name)
{
    b.wait();
    clock_type::time_point tp1 = clock_type::now();
	int total = 0;
	for(int i = 0; i<NUM_ATTEMPTS;)
	{
        int value;
		if(c.try_pop(value))
            ++i;
        ++total;
	}

    clock_type::time_point tp2 = clock_type::now();

	boost::mutex::scoped_lock l(g_log_guard);
	cout << name << " pop: " << tp2 - tp1 << " total pops: " << total << endl;
}

template<typename Container>
void measured_push_proc(Container& c, boost::barrier& b, const char* name)
{
    b.wait();
    clock_type::time_point tp1 = clock_type::now();
	for(int i = 0; i<NUM_ATTEMPTS; ++i)
		c.push(i);

    clock_type::time_point tp2 = clock_type::now();
	boost::mutex::scoped_lock l(g_log_guard);
	cout << name << " push: " << tp2 - tp1 << endl;
}

template<typename Container>
void do_test(const char* name, int push_threads, int pop_threads)
{ 
    Container c;
    boost::barrier b(push_threads + pop_threads);

    std::vector<boost::thread> thrs;
    for(int i = 0; i<push_threads; ++i)
        thrs.push_back(boost::thread(&measured_push_proc<Container>, std::ref(c), std::ref(b), name));

    for(int i = 0; i<pop_threads; ++i)
        thrs.push_back(boost::thread(&measured_pop_proc<Container>, std::ref(c), std::ref(b), name));

    for(int i = 0; i<push_threads + pop_threads; ++i)
        thrs[i].join();
}

int main(int argc, char* argv[])
{
    //do_spsc_test<lf_spsc_queue<int>>("lf_spsc_queue spsc");

    do_test<lb_queue<int>>("lb_queue spsc", 2, 2);
    do_test<lb_fg_queue<int, tcl::allocators::fixed_allocator<int, 10000>>>("lb_fg_queue spsc", 2, 2);

    //do_spsc_test<lf_mpmc_queue<int>>("lb_fg_queue spsc");

    do_test<lb_stack<int>>("lb_stack", 2, 2);
    do_test<lf_stack_hp<int, tcl::allocators::fixed_allocator<int, 10000>>>("lf_stack_hp", 2, 2);
    do_test<lf_stack_refcnt<int, tcl::allocators::fixed_allocator<int, 10000>>>("lf_stack_refcnt", 2, 2);
	return 0;
}
