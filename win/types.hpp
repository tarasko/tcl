#pragma once

#ifdef _WIN32

#include <tcl/unique_res.hpp>

#include <boost/system/error_code.hpp>

#include <Windows.h>

namespace tcl { namespace win {

using ::boost::system::error_code;

typedef TCL_UNIQUE_RES(HANDLE, NULL, CloseHandle) handle;
typedef TCL_UNIQUE_RES(HANDLE, INVALID_HANDLE_VALUE, CloseHandle) handle_ihv;

}}

#endif                                                      // #ifdef _WIN32
