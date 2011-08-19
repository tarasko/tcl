#pragma once

namespace tcl { namespace rll {

/// @brief Basic interface for action objects.
class CAction 
{
public:
    virtual double GetData() const = 0;
};

}}
