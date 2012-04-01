#include <tcl/static_xtor.hpp>

#include <boost/test/auto_unit_test.hpp>
#include <vector>


using namespace tcl;

namespace {

std::vector<int> g_registred_ids;

class one : public tcl::static_xtor<one>
{
    friend class tcl::static_xtor<one>::access;
    static void static_ctor()
    {
        g_registred_ids.push_back(1);
    }
    static void static_dtor()
    {
    }
};

class two : public tcl::static_xtor<two>
{
    friend class tcl::static_xtor<two>::access;
    static void static_ctor()
    {
        g_registred_ids.push_back(2);
    }
    static void static_dtor()
    {
    }
};

}                                                           // anonymous namespace

BOOST_AUTO_TEST_CASE(static_xtor_test)
{
    BOOST_CHECK_EQUAL(g_registred_ids.size(), size_t(2));
    BOOST_CHECK_EQUAL(g_registred_ids[0], 1);
    BOOST_CHECK_EQUAL(g_registred_ids[1], 2);
}
