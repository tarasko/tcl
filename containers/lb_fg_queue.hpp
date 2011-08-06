#pragma once

namespace tcl { namespace containers {

template<typename T>
struct lb_fg_queue_node
{
    lb_fg_queue_node() : next_(0)
    {
    }

    T data_;
    lb_fg_queue_node* next_;
};

template<typename T, typename Allocator = std::allocator<T>>
class lb_fg_queue : Allocator::rebind<lb_fg_queue_node>::other
{
    typedef typename Allocator::rebind<lb_fg_queue_node>::other node_allocator;
    typedef lb_fg_queue_node node;

public:
    lb_fg_queue()
    : head_(node_allocator.allocate(1))
    , tail_(head_)
    {
        // We have just inserted dummy empty node
    }

    void push(const T& data)
    {
        std::auto_ptr<node> new_tail(new node);
        std::shared_ptr<T> new_data = std::make_shared<T>(data);

        boost::mutex::scoped_lock l(tail_guard_);

        tail_->data_ = std::move(new_data);
        tail_->next_ = new_tail.get();
        tail_ = new_tail.release();
    }

    std::shared_ptr<T> try_pop()
    {
        std::shared_ptr<T> res;
        node* old_head;

        {
            boost::mutex::scoped_lock l(head_guard_);
            if (get_tail() == head_)
                return res;

            old_head = head_;
            head_ = head_->next_;
        }

        res = std::move(old_head->data_);
        delete old_head;

        return res;
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
