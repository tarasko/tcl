#pragma once

#include <memory>

namespace tcl { namespace containers {

static_assert (sizeof(void*) == 4, "lf_stack_refcnt should be lockfree");

template<typename T>
class lf_stack_refcnt
{
	struct node;

	struct counted_node_ptr
	{
		counted_node_ptr() : external_count_(0), node_(0)
		{}
		counted_node_ptr(const T& val)
			: external_count_(1), node_(new node(val))
		{};

		int external_count_;
		node* node_;
	};

	struct node
	{
		node(const T& val)
			: internal_count_(0), ptr_(std::make_shared<T>(val))
		{}

		boost::atomic_int internal_count_;
		std::shared_ptr<T> ptr_;
		counted_node_ptr next_;
	};

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
	void push(const T& val)
	{
		counted_node_ptr new_head(val);
		new_head.node_->next_ = head_.load(boost::memory_order_relaxed);
		while (!head_.compare_exchange_weak(new_head.node_->next_,
											new_head,
											boost::memory_order_release,
											boost::memory_order_relaxed));
	}

	std::shared_ptr<T> try_pop()
	{
		counted_node_ptr old_head = head_.load(boost::memory_order_relaxed);

		for(;;)
		{
			// lock head
			increase_head_count(old_head);
			node* ptr = old_head.node_;

			if (!ptr)
				return std::shared_ptr<T>();

			if (head_.compare_exchange_strong(old_head, ptr->next_, boost::memory_order_relaxed))
			{
				std::shared_ptr<T> res;
				res.swap(ptr->ptr_);

				const int count_increase = old_head.external_count_ - 2;
				if (ptr->internal_count_.fetch_add(count_increase, boost::memory_order_release) == -count_increase)
					delete ptr;

				return res;
			}
			else if (ptr->internal_count_.fetch_add(-1, boost::memory_order_relaxed) == 1)
				boost::atomic_thread_fence(boost::memory_order_release);
				delete ptr;
		}
	}

private:
	boost::atomic<counted_node_ptr> head_;
};

}}
