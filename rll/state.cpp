#include "state.hpp"

#include <algorithm>
#include <cassert>

namespace tcl { namespace rll {

state::state(size_t signals_num)
    : signals_(std::make_shared<vector_rllt>(signals_num))
{
}

const rll_type& state::operator[](size_t idx) const
{
    return signals_->operator[](idx);
}

rll_type& state::operator[](size_t idx)
{
    return signals_->operator[](idx);
}

/// @brief Return number of state signals
size_t state::signals_num() const
{
    return signals_->size();
}

vector_rllt_csp state::get_internal_rep() const
{
    return signals_;
}

state state::clone() const
{
    vector_rllt_sp signals = std::make_shared<vector_rllt>(*signals_);
    return state(signals);
}

state::state(const vector_rllt_sp& signals)
    : signals_(signals)
{
}

bool operator==(const state& f, const state& s)
{
    return std::equal(f.signals_->begin(), f.signals_->end(), s.signals_->begin());
}

// -------------- class state_with_reserved_action -------------------

state_with_reserved_action::state_with_reserved_action(size_t signals_num)
    : signals_(std::make_shared<vector_rllt>(signals_num + 1))
{
}

const rll_type& state_with_reserved_action::operator[](size_t idx) const
{
    assert(idx < signals_->size() - 1);
    return signals_->operator[](idx);
}

rll_type& state_with_reserved_action::operator[](size_t idx)
{
    assert(idx < signals_->size() - 1);
    return signals_->operator[](idx);
}

/// @brief Return number of state signals
size_t state_with_reserved_action::signals_num() const
{
    return signals_->size() - 1;
}

vector_rllt_csp state_with_reserved_action::get_internal_rep(rll_type action)
{
    signals_->back() = action;
    return signals_;
}

state_with_reserved_action state_with_reserved_action::clone() const
{
    vector_rllt_sp signals = std::make_shared<vector_rllt>(*signals_);
    return state_with_reserved_action(signals);
}

state_with_reserved_action::state_with_reserved_action(const vector_rllt_sp& signals)
    : signals_(signals)
{
}

bool operator==(const state_with_reserved_action& f, const state_with_reserved_action& s)
{
    return std::equal(f.signals_->begin(), f.signals_->end(), s.signals_->begin());
}

}}
