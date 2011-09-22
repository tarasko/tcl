#include "state.hpp"

#include <algorithm>

namespace tcl { namespace rll {

bool operator==(const state& f, const state& s)
{
    return std::equal(f.signals_->begin(), f.signals_->end(), s.signals_->begin());
}

bool operator==(const state_with_reserved_action& f, const state_with_reserved_action& s)
{
    return std::equal(f.signals_->begin(), f.signals_->end(), s.signals_->begin());
}

}}
