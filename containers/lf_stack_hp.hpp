#pragma once

#include "hazard_pointers.hpp"

#include <tcl/allocators/construct_destroy.hpp>

#include <boost/atomic.hpp>

#include <memory>

namespace tcl { namespace containers {

namespace detail {

// Node type for stack
template<typename T>
struct lf_stack_hp_node
{
    lf_stack_hp_node(const T& value) : value_(value), next_(0)
    {}

    T value_;
    lf_stack_hp_node* next_;
};

}

/// \brief Lock-free stack, that use hazard pointers to reclaim free node.
template<typename T, class Allocator = std::allocator<T>>
class lf_stack_hp : Allocator::template rebind<detail::lf_stack_hp_node<T> >::other
{
    typedef detail::lf_stack_hp_node<T> node;
    typedef typename Allocator::template rebind<node>::other node_allocator;

    typedef hazard_pointers<node, 10> hazard_pointers_type;

    lf_stack_hp(const lf_stack_hp&);
    lf_stack_hp& operator=(const lf_stack_hp&);

public:
    lf_stack_hp(const Allocator& allocator = Allocator()) 
        : node_allocator(allocator)
        , head_(0) 
    {
    }

    void push(const T& value)
    {
        node* new_node = allocators::construct(*(node_allocator*)this, value);
        new_node->next_ = head_.load(boost::memory_order_relaxed);
        while(!head_.compare_exchange_weak(new_node->next_, new_node, boost::memory_order_release));
    }

    bool try_pop(T& result)
    {
        node* old_head = head_.load(boost::memory_order_relaxed);
        {
            typename hazard_pointers_type::scoped_allocator sa(hps_);
            do {
                sa.ref().store(old_head, boost::memory_order_relaxed);
            } while(old_head && !head_.compare_exchange_weak(old_head, old_head->next_, boost::memory_order_release));
        }

        if (old_head)
        {
            result = std::move(old_head->value_);

            if (hps_.outstanding_hp_for(old_head))
                allocators::destroy(*(node_allocator*)this, old_head);
            else
                hps_.reclaim_later(old_head, (node_allocator&)*this);

            return true;
        }

        return false;
    }

private:
    hazard_pointers_type hps_;
    boost::atomic<node*> head_;
};

}}
