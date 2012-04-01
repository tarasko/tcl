#pragma once

#ifdef _WIN32

#include <tcl/win/types.hpp>

namespace tcl { namespace win { namespace security {

/// \brief Open process security token.
/// Throw exception boost::system_error if failed.
///
/// \param pid - Process ID
/// \param desired_access - Desired access rights
/// \return Return process security token handle.
handle open_process_token(DWORD pid, DWORD desired_access);

/// \brief Open process security token.
/// Return empty handle and assign error to ec if failed.
///
/// \param pid - Process ID
/// \return Return process security token handle.
handle open_process_token(DWORD pid, DWORD desired_access, error_code& ec);

}}}                                                         // namespace tcl, win, security

#endif                                                      // #ifdef _WIN32
