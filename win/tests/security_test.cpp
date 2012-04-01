#define BOOST_TEST_MODULE tcl.win

#include <tcl/win/security.hpp>

#include <boost/test/auto_unit_test.hpp>
#include <boost/system/system_error.hpp>

using namespace ::tcl::win;
using namespace ::tcl::win::security;

BOOST_AUTO_TEST_CASE(open_process_token_test)
{
    DWORD pid = GetCurrentProcessId();
    handle token = open_process_token(pid, TOKEN_QUERY);

    BOOST_CHECK(token);

    BOOST_CHECK_THROW((open_process_token(-1, TOKEN_QUERY)), boost::system::system_error);
    BOOST_CHECK_THROW((open_process_token(pid, -1)), boost::system::system_error);
}
