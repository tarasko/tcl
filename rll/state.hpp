#pragma once

#include "rll_fwd.hpp"

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
    state() {}

    /// @brief Construct state with specified number of signals
    state(size_t signals_num);

    // Accessors
    const rll_type& operator[](size_t idx) const;
    rll_type& operator[](size_t idx);

    /// @brief Return number of state signals.
    size_t signals_num() const;
    
    /// @brief Return internal representation for state    
    vector_rllt_csp get_internal_rep() const;

    /// @brief Spawn new state by makeing a deep copy of current one.
    state clone() const;

private:
    friend bool operator==(const state& f, const state& s);

    state(const vector_rllt_sp& signals);

private:
    vector_rllt_sp signals_;  //!< Signals
};

/// @brief Represent agent state with reserved slot for action.
/// @copy state.
class state_with_reserved_action
{
public:
    state_with_reserved_action() {}

    /// @brief Construct state with specified number of signals
    state_with_reserved_action(size_t signals_num);

    // Accessors
    const rll_type& operator[](size_t idx) const;
    rll_type& operator[](size_t idx);

    /// @brief Return number of state signals.
    size_t signals_num() const;
    
    /// @brief Return internal representation for state.
    vector_rllt_csp get_internal_rep(rll_type action);

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

}}
