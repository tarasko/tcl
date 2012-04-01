#include <tcl/unique_res.hpp>

#include <boost/test/auto_unit_test.hpp>
#include <iostream>

using namespace std;

typedef TCL_UNIQUE_RES(void*, 0, free) arr_t;

template<typename ResType>
void test_ctor()
{
    ResType a1;
    BOOST_CHECK(!a1);
    BOOST_CHECK_EQUAL(a1.get(), (void*)0);
    void* tmp = malloc(16);
    ResType a2(tmp);
    if(a2)
    {
    }
    else
    {
        BOOST_CHECK(false);
    }
    BOOST_CHECK_EQUAL(a2.get(), tmp);
}

arr_t create_arr1()
{
    arr_t arr = malloc(16);
    return boost::move(arr);
}
arr_t create_arr2()
{
    return malloc(16);              
    return arr_t(malloc(16)); // check syntax
}

BOOST_AUTO_TEST_CASE(unique_res_ctor_test)
{
    test_ctor<arr_t>();

    // check constructor and move constructor
    arr_t arr1 = create_arr1();
    arr_t arr2 = create_arr2();
    BOOST_CHECK(arr1);
    BOOST_CHECK(arr2);

    arr_t arr3 = boost::move(arr1);
    BOOST_CHECK(!arr1);
}

BOOST_AUTO_TEST_CASE(unique_res_assign_test)
{
    // check move assignment
    arr_t arr4 = malloc(16);
    arr_t arr5;
    arr5 = boost::move(arr4);
    BOOST_CHECK(!arr4);
    BOOST_CHECK(!!arr5);
}

void assign_array(void* ptr, void** pptr)
{
    *pptr = ptr;
}

// Test ptr method
BOOST_AUTO_TEST_CASE(res_ptr_test)
{
    arr_t a;
    BOOST_CHECK(!a);

    void* ptr = malloc(16);
    assign_array(ptr, a.ptr());
    BOOST_CHECK_EQUAL(ptr, a.get());
}
