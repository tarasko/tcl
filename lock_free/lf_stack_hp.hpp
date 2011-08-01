#pragma once

#include "hazard_pointers.hpp"

#include <memory>

namespace tcl { namespace lock_free {

template<typename T>
class lf_stack_hp
{
	struct node
	{
		node(const T& value) : value_(std::make_shared<T>(value)), next_(0)
		{}

		std::shared_ptr<T> value_;
		node* next_;
	};

	typedef hazard_pointers<node, 10> hazard_pointers_type;

	lf_stack_hp(const lf_stack_hp&);
	lf_stack_hp& operator=(const lf_stack_hp&);

public:
	lf_stack_hp() : head_(0) {}

	void push(const T& value)
	{
		node* new_node = new node(value);
		new_node->next_ = head_.load(boost::memory_order_relaxed);
		while(!head_.compare_exchange_weak(new_node->next_, new_node, boost::memory_order_release));
	}

	std::shared_ptr<T> try_pop()
	{
		node* old_head = head_.load(boost::memory_order_relaxed);
		{
			typename hazard_pointers_type::scoped_allocator sa(hps_);
			do {
				sa.ref().store(old_head, boost::memory_order_relaxed);
			} while(old_head && !head_.compare_exchange_weak(old_head, old_head->next_, boost::memory_order_release));
		}

		std::shared_ptr<T> ret;
		if (old_head)
		{
			ret.swap(old_head->value_);

			if (hps_.outstanding_hp_for(old_head))
				delete old_head;
			else
				hps_.reclaim_later(old_head);
		}

		return ret;
	}

private:
	hazard_pointers_type hps_;
	boost::atomic<node*> head_;
};

}}
