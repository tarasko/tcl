#define BOOST_TEST_MODULE tcl

#include <tcl/weak_ptr_closure.hpp>

#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/test/unit_test.hpp>
using namespace tcl;

struct A 
{
    A() : f_called(false) {}
    void f(int p1, int p2)
    {
        f_called = true;
        BOOST_CHECK_EQUAL(p1, 1);
        BOOST_CHECK_EQUAL(p2, 2);
    }
    bool f_called;
};

BOOST_AUTO_TEST_CASE( weak_ptr_closure_test )
{
    auto boost_sptr = boost::make_shared<A>();
    boost::weak_ptr<A> boost_wptr = boost_sptr;

    weak_ptr_closure(&A::f, boost_sptr)(boost::cref(1), boost::cref(2));
    BOOST_CHECK(boost_sptr->f_called);
    boost_sptr->f_called = false;

    weak_ptr_closure(&A::f, boost_wptr)(boost::cref(1), boost::cref(2));
    BOOST_CHECK(boost_sptr->f_called);
    boost_sptr->f_called = false;

    auto std_sptr = std::make_shared<A>();
    std::weak_ptr<A> std_wptr = std_sptr;

    weak_ptr_closure(&A::f, std_sptr)(std::cref(1), std::cref(2));
    BOOST_CHECK(std_sptr->f_called);
    std_sptr->f_called = false;

    weak_ptr_closure(&A::f, std_wptr)(std::cref(1), std::cref(2));
    BOOST_CHECK(std_sptr->f_called);
    std_sptr->f_called = false;
}

