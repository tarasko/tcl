#pragma once

#include "rll_fwd.hpp"

namespace tcl { namespace rll {

/// @brief Basic interface for action objects.
class CAction 
{
public:
    virtual rll_type GetData() const = 0;
};

}}
