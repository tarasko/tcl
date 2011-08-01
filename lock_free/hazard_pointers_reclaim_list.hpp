#pragma once

#include <boost/atomic.hpp>

#include <functional>
#include <vector>

namespace tcl { namespace lock_free {

struct hazard_pointers_reclaim_node
{
	template<typename T>
	hazard_pointers_reclaim_node(T* ptr)
	: ptr_(ptr)
	, deleter_(&hazard_pointers_reclaim_node::std_deleter<T>)
	{
	}

	template<typename T, typename Deleter>
	hazard_pointers_reclaim_node(T* ptr, Deleter deleter)
	: ptr_(ptr)
	, deleter_(deleter)
	{
	}

	hazard_pointers_reclaim_node(hazard_pointers_reclaim_node&& other)
    : ptr_(other.ptr_)
    , deleter_(std::move(other.deleter_))
    {
    }

	hazard_pointers_reclaim_node& operator=(hazard_pointers_reclaim_node&& other)
    {
        ptr_ = other.ptr_;
        deleter_ = std::move(other.deleter_);

        return *this;
    }

	template<typename T>
	T* ptr() const
	{
		return static_cast<T*>(ptr_);
	}

	void dispose()
	{
		deleter_(ptr_);
		ptr_ = 0;
	}

	void swap(hazard_pointers_reclaim_node& n)
	{
		std::swap(ptr_, n.ptr_);
		std::swap(deleter_, n.deleter_);
	}

private:
	hazard_pointers_reclaim_node();
	hazard_pointers_reclaim_node(const hazard_pointers_reclaim_node&);
	hazard_pointers_reclaim_node& operator=(const hazard_pointers_reclaim_node&);

    template<typename T>
	static void std_deleter(void* ptr)
	{
		delete static_cast<T*>(ptr);
	}

	friend bool operator<(hazard_pointers_reclaim_node& n1, hazard_pointers_reclaim_node& n2)
	{
		return n1.ptr_ < n2.ptr_;
	}

	void* ptr_;
	std::function<void(void*)> deleter_;
};

typedef std::vector<hazard_pointers_reclaim_node> hazard_pointers_reclaim_list;

hazard_pointers_reclaim_list& get_reclaim_list();

}}

namespace std {

template<> inline void swap(
    tcl::lock_free::hazard_pointers_reclaim_node& n1
  , tcl::lock_free::hazard_pointers_reclaim_node& n2
  )
{
	n1.swap(n2);
}

}
