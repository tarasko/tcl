#include "../construct_destroy.hpp"

#include <memory>
#include <iostream>

using namespace std;

struct A
{
    A()
    {
        if (s++ == 10)
            throw 1;

        cout << "A::A\n";
    }

    ~A()
    {
        cout << "A::~A\n";
    }

    static int s;
};

int A::s = 0;

int main(int argc, char* argv[])
{
    std::allocator<short> al;

    short* s1 = tcl::allocators::construct(al, 10);
    tcl::allocators::destroy(al, s1);

    try
    {
        std::allocator<A> al1;
        A* ar = tcl::allocators::construct_array(al1, 20);
        tcl::allocators::destroy_array(al1, ar, 20);
    }
    catch(...)
    {
        cout << "fuck \n";
    }

    return 0;
}
