#include "../alexandrescu/small_obj_allocator.hpp"
#include "../fixed_allocator.hpp"

#include <boost/chrono/chrono.hpp>
#include <boost/chrono/chrono_io.hpp>

#include <vector>
#include <iostream>
#include <functional>

const size_t attempts = 15000;

typedef boost::chrono::steady_clock clock_type;
typedef long double test_type;

using namespace tcl::allocators;

std::vector<test_type*> g_ptrs(attempts);

template<typename Allocator>
void test_al(Allocator& al)
{
    clock_type::time_point tp1 = clock_type::now();

    for(int i = 0; i<attempts; ++i)
    {
        g_ptrs[i] = al.allocate(1);
        *(g_ptrs[i]) = i;
    }

    clock_type::time_point tp2 = clock_type::now();

    for(int i = 0; i<attempts; ++i)
        al.deallocate(g_ptrs[i], 1);

    clock_type::time_point tp3 = clock_type::now();

    std::cout << typeid(Allocator).name() << std::endl 
        << tp2 - tp1 << std::endl << tp3 - tp1 << std::endl;
}

int main(int argc, char* argv[])
{
    fixed_allocator<test_type, attempts> my;
    std::allocator<test_type> def;

    test_al(my);
    test_al(def);

    return 0;
}
