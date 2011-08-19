#pragma once

#include <tcl/allocators/construct_destroy.hpp>

#include <boost/atomic.hpp>

namespace tcl { namespace containers {

namespace detail {

template<typename T>
struct lf_mpmc_queue_node;

template<typename T>
struct lf_mpmc_queue_counted_node_ptr
{
    int external_count_;
    lf_mpmc_queue_node<T>* node_;
};

struct lf_mpmc_queue_node_counter
{
    int internal_count_:30;
    unsigned external_counters_:2;
};

template<typename T>
struct lf_mpmc_queue_node
{
    boost::atomic<T*> data_;
    boost::atomic<lf_mpmc_queue_node_counter> count_;
    lf_mpmc_queue_counted_node_ptr<T> next_;

    lf_mpmc_queue_node()
    {
        lf_mpmc_queue_node_counter new_count;
        new_count.internal_count_ = 0;
        new_count.external_counters_ = 2;
        count_.store(new_count, boost::memory_order_relaxed);

        next_.node_ = 0;
        next_.external_count_ = 0;
    }

    template<typename Allocator>
    void release_ref(Allocator& allocator)
    {
        lf_mpmc_queue_node_counter old_counter = count_.load();
        lf_mpmc_queue_node_counter new_counter;
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
            allocators::destroy(allocator, this);
    }
};

}

template<typename T, typename Allocator>
class lf_mpmc_queue : Allocator::template rebind<detail::lf_mpmc_queue_node<T> >::other
{
    typedef detail::lf_mpmc_queue_node<T> node;
    typedef detail::lf_mpmc_queue_counted_node_ptr<T> counted_node_ptr;
    typedef detail::lf_mpmc_queue_node_counter node_counter;

    typedef typename Allocator::template rebind<node>::other node_allocator;

public:
    lf_mpmc_queue(const Allocator& allocator = Allocator())
        : node_allocator(allocator)
    {
        counted_node_ptr new_node;
        new_node.external_count_ = 1;
        new_node.node_ = allocators::construct(*(node_allocator*)this);
        head_.store(new_node, boost::memory_order_relaxed);
        tail_.store(new_node, boost::memory_order_relaxed);
    }

    ~lf_mpmc_queue()
    {
        T dummy;
        while(try_pop(dummy));
        allocators::destroy(
            *(node_allocator*)this
          , head_.load(boost::memory_order_relaxed).node_
          );
    }

    bool try_pop(T& result)
    {
        counted_node_ptr old_head = head_.load(boost::memory_order_relaxed);
        for(;;)
        {
            increase_external_count(head_, old_head);
            node* const ptr = old_head.node_;
            if (ptr == tail_.load().node_)
                return false;

            if (head_.compare_exchange_strong(old_head, ptr->next_))
            {
                T* const res = ptr->data_.exchange(0);
                free_external_count(old_head);
                result = std::move(*res);
                delete res;
                return true;
            }

            ptr->release_ref(*(node_allocator*)this);
        }
    }

    void push(const T& new_value)
    {
        std::unique_ptr<T> new_data(new T(new_value));
        counted_node_ptr new_next;
        new_next.node_ = allocators::construct(*(node_allocator*)this);
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
            old_tail.node_->release_ref(*(node_allocator*)this);
        }
    }

private:

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

    void free_external_count(counted_node_ptr& old_node_ptr)
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
            allocators::destroy(*(node_allocator*)this, ptr);
    }

    lf_mpmc_queue(const lf_mpmc_queue&);
    lf_mpmc_queue& operator=(const lf_mpmc_queue&);

private:
    boost::atomic<counted_node_ptr> head_;
    boost::atomic<counted_node_ptr> tail_;
};

}}
