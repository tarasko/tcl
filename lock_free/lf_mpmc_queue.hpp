#pragma once

namespace tcl { namespace lock_free {

template<typename T>
class lf_mpmc_queue
{
public:
    lf_mpmc_queue()
    {
        counted_node_ptr new_node;
        new_node.external_count_ = 1;
        new_node.node_ = new node;
        head_.store(new_node, boost::memory_order_relaxed);
        tail_.store(new_node, boost::memory_order_relaxed);
    }

    ~lf_mpmc_queue()
    {
        while(try_pop());
        delete head_.load(boost::memory_order_relaxed).node_;
    }

    std::unique_ptr<T> try_pop()
    {
        counted_node_ptr old_head = head_.load(boost::memory_order_relaxed);
        for(;;)
        {
            increase_external_count(head_, old_head);
            node* const ptr = old_head.node_;
            if (ptr == tail_.load().node_)
                return std::unique_ptr<T>();

            if (head_.compare_exchange_strong(old_head, ptr->next_))
            {
                T* const res = ptr->data_.exchange(0);
                free_external_count(old_head);
                return std::unique_ptr<T>(res);
            }

            ptr->release_ref();
        }
    }

    void push(const T& new_value)
    {
        std::unique_ptr<T> new_data(new T(new_value));
        counted_node_ptr new_next;
        new_next.node_ = new node;
        new_next.external_count_ = 1;
        counted_node_ptr old_tail = tail_.load();

        for(;;)
        {
            increase_external_count(tail_, old_tail);

            T* old_data = 0;
            if (old_tail.node_->data_.compare_exchange_strong(old_data, new_data.get()))
            {
                old_tail.node_->next_ = new_next;
                old_tail = tail_.exchange(new_next);
                free_external_count(old_tail);
                new_data.release();
                break;
            }
            old_tail.node_->release_ref();
        }
    }

private:
    struct node;
    struct counted_node_ptr
    {
        int external_count_;
        node* node_;
    };

    struct node_counter
    {
        int internal_count_:30;
        unsigned external_counters_:2;
    };

    struct node
    {
        boost::atomic<T*> data_;
        boost::atomic<node_counter> count_;
        counted_node_ptr next_;

        node()
        {
            node_counter new_count;
            new_count.internal_count_ = 0;
            new_count.external_counters_ = 2;
            count_.store(new_count, boost::memory_order_relaxed);

            next_.node_ = 0;
            next_.external_count_ = 0;
        }

        void release_ref()
        {
            node_counter old_counter = count_.load();
            node_counter new_counter;
            do
            {
                new_counter = old_counter;
                --new_counter.internal_count_;
            }
            while (!count_.compare_exchange_strong(
                old_counter,
                new_counter,
                boost::memory_order_acquire,
                boost::memory_order_relaxed));

            if (!new_counter.internal_count_ && !new_counter.external_counters_)
            {
                delete this;
            }
        }
    };

    static void increase_external_count(
        boost::atomic<counted_node_ptr>& counter
      , counted_node_ptr& old_counter)
    {
        counted_node_ptr new_counter;

        do
        {
            new_counter = old_counter;
            ++new_counter.external_count_;
        }
        while(!counter.compare_exchange_strong(
            old_counter,
            new_counter,
            boost::memory_order_acquire,
            boost::memory_order_relaxed));

        old_counter.external_count_ = new_counter.external_count_;
    }

    static void free_external_count(counted_node_ptr& old_node_ptr)
    {
        node* const ptr = old_node_ptr.node_;
        int const count_increase = old_node_ptr.external_count_ - 2;
        node_counter old_counter = ptr->count_.load(boost::memory_order_relaxed);
        node_counter new_counter;

        do
        {
            new_counter = old_counter;
            --new_counter.external_counters_;
            new_counter.internal_count_ += count_increase;
        }
        while(!ptr->count_.compare_exchange_strong(
            old_counter,
            new_counter,
            boost::memory_order_acquire,
            boost::memory_order_release));

        if(!new_counter.internal_count_ && !new_counter.external_counters_)
        {
            delete ptr;
        }
    }

    lf_mpmc_queue(const lf_mpmc_queue&);
    lf_mpmc_queue& operator=(const lf_mpmc_queue&);

private:
    boost::atomic<counted_node_ptr> head_;
    boost::atomic<counted_node_ptr> tail_;
};

}}
