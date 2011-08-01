#pragma once

namespace tcl { namespace lock_free {

template<typename T>
class lf_spsc_queue
{
public:
    lf_spsc_queue()
    : head_(new node)
    , tail_(head_)
    {
        // We have just inserted dummy empty node
    }

    void push(const T& data)
    {
        std::auto_ptr<node> new_tail(new node);

        node* old_tail = tail_.load(boost::memory_order_relaxed);
        old_tail->data_ = std::make_shared<T>(data);
        old_tail->next_ = new_tail.get();

        tail_.store(new_tail.release(), boost::memory_order_release);
    }

    std::shared_ptr<T> try_pop()
    {
        std::shared_ptr<T> res;
        if (head_ == tail_.load(boost::memory_order_acquire))
            return res;

        // Nobody can modify head_ here, so we are guranteed that
        // head_ != tail_ until the fuction end

        node* old_head = head_;
        head_ = head_->next_;
        res = std::move(old_head->data_);
        delete old_head;

        return res;
    }

private:
    struct node
    {
        node() : next_(0)
        {
        }

        std::shared_ptr<T> data_;
        node* next_;
    };

    node* head_;
    boost::atomic<node*> tail_;
};

}}
