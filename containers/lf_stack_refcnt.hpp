#pragma once

#include <tcl/allocators/construct_destroy.hpp>

#include <boost/atomic.hpp>

#include <memory>

namespace tcl { namespace containers {

template<typename T>
struct lf_stack_refcnt_node;

template<typename T>
struct lf_stack_refcnt_counted_node_ptr
{
	int external_count_;
	lf_stack_refcnt_node<T>* node_;
};

template<typename T>
struct lf_stack_refcnt_node
{
	lf_stack_refcnt_node(const T& val)
		: internal_count_(0)
        , value_(val)
	{}

	boost::atomic_int internal_count_;
	T value_;
	lf_stack_refcnt_counted_node_ptr<T> next_;
};

template<typename T, typename Allocator = std::allocator<T> >
class lf_stack_refcnt : Allocator::template rebind<lf_stack_refcnt_node<T> >::other
{
    typedef lf_stack_refcnt_node<T> node;
    typedef lf_stack_refcnt_counted_node_ptr<T> counted_node_ptr;
    typedef typename Allocator::template rebind<node>::other node_allocator;

	void increase_head_count(counted_node_ptr& old_counter)
	{
		counted_node_ptr new_counter;

		do {
			new_counter = old_counter;
			++new_counter.external_count_;
		} while(!head_.compare_exchange_weak(old_counter,
											new_counter,
											boost::memory_order_acquire,
											boost::memory_order_relaxed));

		old_counter.external_count_ = new_counter.external_count_;
	}

public:
    lf_stack_refcnt(const Allocator& allocator = Allocator())
        : node_allocator(allocator)
    {
        counted_node_ptr dummy;
        dummy.external_count_ = 1;
        dummy.node_ = 0;
        head_.store(dummy, boost::memory_order_relaxed);
    }

	void push(const T& val)
	{
		counted_node_ptr new_head;
        new_head.external_count_ = 1;
        new_head.node_ = allocators::construct(*(node_allocator*)this, val);
		new_head.node_->next_ = head_.load(boost::memory_order_relaxed);

		while (!head_.compare_exchange_weak(new_head.node_->next_,
											new_head,
											boost::memory_order_release,
											boost::memory_order_relaxed));
	}

	bool try_pop(T& result)
	{
		counted_node_ptr old_head = head_.load(boost::memory_order_relaxed);

		for(;;)
		{
			// lock head
			increase_head_count(old_head);
			node* ptr = old_head.node_;

			if (!ptr)
				return false;

			if (head_.compare_exchange_strong(old_head, ptr->next_, boost::memory_order_relaxed))
			{
                result = std::move(ptr->value_);

				const int count_increase = old_head.external_count_ - 2;
				if (ptr->internal_count_.fetch_add(count_increase, boost::memory_order_release) == -count_increase)
                    allocators::destroy(*(node_allocator*)this, ptr);

				return true;
			}
			else if (ptr->internal_count_.fetch_add(-1, boost::memory_order_relaxed) == 1)
				boost::atomic_thread_fence(boost::memory_order_release);
                allocators::destroy(*(node_allocator*)this, ptr);
		}
	}

private:
	boost::atomic<counted_node_ptr> head_;
};

}}
