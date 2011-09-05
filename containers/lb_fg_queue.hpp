#pragma once

#include <tcl/allocators/construct_destroy.hpp>

#include <memory>

namespace tcl { namespace containers {

namespace detail {

template<typename T>
struct lb_fg_queue_node
{
    lb_fg_queue_node(const T& data) : data_(data), next_(0)
    {
    }

    T data_;
    lb_fg_queue_node* next_;
};

}

template<typename T, typename Allocator = std::allocator<T> >
class lb_fg_queue : Allocator::template rebind<detail::lb_fg_queue_node<T> >::other
{
    typedef typename Allocator::template rebind<detail::lb_fg_queue_node<T> >::other node_allocator;
    typedef detail::lb_fg_queue_node<T> node;

public:
    lb_fg_queue(const Allocator& allocator = Allocator())
    : node_allocator(allocator)
    , head_(node_allocator::allocate(1))
    , tail_(head_)
    {
        // We have just inserted dummy empty node
    }

    void push(const T& data)
    {
        node* new_tail = allocators::construct(*(node_allocator*)this, data);

        boost::mutex::scoped_lock l(tail_guard_);
        tail_->next_ = new_tail;
        tail_ = new_tail;
    }

    bool try_pop(T& result)
    {
        node* old_head;

        {
            boost::mutex::scoped_lock l(head_guard_);
            if (get_tail() == head_)
                return false;

            old_head = head_;
            head_ = head_->next_;
        }

        result = std::move(old_head->data_);
        allocators::destroy(*(node_allocator*)this, old_head);

        return true;
    }

private:
    node* get_tail()
    {
        boost::mutex::scoped_lock l(tail_guard_);
        return tail_;
    }

    node* head_;
    boost::mutex head_guard_;

    node* tail_;
    boost::mutex tail_guard_;
};

}}
