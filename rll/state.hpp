#pragma once

#include "rll_fwd.hpp"

#include <cassert>
#include <algorithm>

namespace tcl { namespace rll {

/// @brief Represent agent state.
/// Agent state consist of fixed number of signal values. Application is supposed to 
/// define number of signals in state, and give some meaning to this signals. 
/// For example, 3x3 tic-tac-toe game could have 9 state signals each one represent
/// corresponding square on game field. If square is empty then signal will have value 0, 
/// if X - then 1, if O - then 2.
/// @c state contains a shared pointer to internal representation, so copying of state object 
/// will just increase reference counter. To make deep copy use @clone method.
class state 
{
public:
    /// @brief Construct empty state.
    /// @todo Currently it needs only to fit to std::pair. Should be removed
    state();

    /// @brief Construct state with specified number of signals
    state(size_t signals_num);

    /// @brief Construct state from internal state representation.
    /// Doesn`t perform deep copy
    state(const vector_rllt_sp& signals);

    /// @name State signal accessors
    /// @{
    const rll_type& operator[](size_t idx) const;
    rll_type& operator[](size_t idx);
    /// @}

    /// @brief Return number of state signals.
    size_t signals_num() const;
    
    /// @name Return internal representation for state    
    ///@{
    vector_rllt_csp get_internal_rep() const;
    vector_rllt_sp get_internal_rep();
    ///@}

    /// @brief Spawn new state by makeing a deep copy of current one.
    state clone() const;

private:
    friend bool operator==(const state& f, const state& s);

private:
    vector_rllt_sp signals_;  //!< Signals
};

/// @brief Represent agent state with reserved slot for action.
/// @copy state.
class state_with_reserved_action
{
public:
    state_with_reserved_action();

    /// @brief Construct state with specified number of signals
    state_with_reserved_action(size_t signals_num);

    // Accessors
    const rll_type& operator[](size_t idx) const;
    rll_type& operator[](size_t idx);

    /// @brief Return number of state signals.
    size_t signals_num() const;
    
    /// @brief Return internal representation for state.
    vector_rllt_csp get_internal_rep(rll_type action) const;
    vector_rllt_sp get_internal_rep(rll_type action);

    /// @brief Spawn new state by makeing a deep copy of current one.
    state_with_reserved_action clone() const;

private:
    friend bool operator==(
        const state_with_reserved_action& f
      , const state_with_reserved_action& s
      );

    state_with_reserved_action(const vector_rllt_sp& signals);

private:
    vector_rllt_sp signals_;  //!< Signals
};

// class state

inline state::state()
{
}

inline state::state(size_t signals_num)
    : signals_(std::make_shared<vector_rllt>(signals_num))
{
}

inline state::state(const vector_rllt_sp& signals)
    : signals_(signals)
{
}

inline const rll_type& state::operator[](size_t idx) const
{
    return signals_->operator[](idx);
}

inline rll_type& state::operator[](size_t idx)
{
    return signals_->operator[](idx);
}

inline size_t state::signals_num() const
{
    return signals_->size();
}

inline vector_rllt_csp state::get_internal_rep() const
{
    return signals_;
}

inline vector_rllt_sp state::get_internal_rep() 
{
    return signals_;
}

inline state state::clone() const
{
    vector_rllt_sp signals = std::make_shared<vector_rllt>(*signals_);
    return state(signals);
}

inline bool operator==(const state& f, const state& s)
{
    return std::equal(f.signals_->begin(), f.signals_->end(), s.signals_->begin());
}

// class state_with_reserved_action

state_with_reserved_action::state_with_reserved_action()
{
}

inline state_with_reserved_action::state_with_reserved_action(size_t signals_num)
    : signals_(std::make_shared<vector_rllt>(signals_num + 1))
{
}

inline const rll_type& state_with_reserved_action::operator[](size_t idx) const
{
    assert(idx < signals_->size() - 1);
    return signals_->operator[](idx);
}

inline rll_type& state_with_reserved_action::operator[](size_t idx)
{
    assert(idx < signals_->size() - 1);
    return signals_->operator[](idx);
}

/// @brief Return number of state signals
inline size_t state_with_reserved_action::signals_num() const
{
    return signals_->size() - 1;
}

inline vector_rllt_csp state_with_reserved_action::get_internal_rep(rll_type action) const
{
    signals_->back() = action;
    return signals_;
}

inline state_with_reserved_action state_with_reserved_action::clone() const
{
    vector_rllt_sp signals = std::make_shared<vector_rllt>(*signals_);
    return state_with_reserved_action(signals);
}

inline state_with_reserved_action::state_with_reserved_action(const vector_rllt_sp& signals)
    : signals_(signals)
{
}

inline bool operator==(const state_with_reserved_action& f, const state_with_reserved_action& s)
{
    return std::equal(f.signals_->begin(), f.signals_->end(), s.signals_->begin());
}

}}
