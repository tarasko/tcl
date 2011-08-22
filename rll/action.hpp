#pragma once

#include "types.hpp"

namespace tcl { namespace rll {

/// @brief Basic interface for action objects.
class CAction 
{
public:
    virtual rll_type GetData() const = 0;
};

}}
