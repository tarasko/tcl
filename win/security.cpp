/// \todo Move on boost tuple for compatibility when it start to support 
/// moveable only types. Currently use std::tuple and std::move

#include "security.hpp"

#include <boost/system/system_error.hpp>
#include <boost/system/windows_error.hpp>

#include <tuple>

namespace tcl { namespace win { namespace security {

namespace {

template<typename Tuple>
inline Tuple& assign_le(Tuple& t, const char* what)
{
    std::get<1>(t) = boost::system::error_code(::GetLastError(), boost::system::system_category());
    std::get<2>(t) = what;
    return t;
}

template<typename Tuple>
inline typename std::tuple_element<0, Tuple>::type
throw_on_error(Tuple&& t)
{
    if (!std::get<1>(t))
        return std::move(std::get<0>(t));

    throw boost::system::system_error(std::get<1>(t), std::get<2>(t));
}

template<typename Tuple>
inline typename std::tuple_element<0, Tuple>::type 
assign_ec(Tuple&& t, error_code& ec)
{
    ec = std::get<1>(t);
    return std::move(std::get<0>(t));
}

inline std::tuple<handle, error_code, const char*>
open_process_token_impl(DWORD pid, DWORD desired_access)
{
    std::tuple<handle, error_code, const char*> res;

    handle hprocess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);

    if (!hprocess) 
        assign_le(res, "unable to open process handle");
    else if (!OpenProcessToken(hprocess.get(), desired_access, std::get<0>(res).ptr()))
        assign_le(res, "unable to open token handle");

    return std::move(res);
}

}                                                           // anon namespace

handle open_process_token(DWORD pid, DWORD desired_access)
{
    return throw_on_error(open_process_token_impl(pid, desired_access));
}

handle open_process_token(DWORD pid, DWORD desired_access, boost::system::error_code& ec)
{
    return assign_ec(open_process_token_impl(pid, desired_access), ec);
}

}}}
