#include <tcl/safe_bool.hpp>

#include <boost/test/auto_unit_test.hpp>

using namespace tcl;

namespace {

template<bool ExpectedResult>
struct tester : safe_bool< tester<ExpectedResult> >
{
    bool boolean_test() const
    {
        return ExpectedResult;
    }
};

}                                                           // anonymous namespace

BOOST_AUTO_TEST_CASE(safe_bool_test)
{
    BOOST_CHECK(!tester<false>());
    BOOST_CHECK(tester<true>());
}
