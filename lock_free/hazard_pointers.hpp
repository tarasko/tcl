#pragma once

#include "hazard_pointers_reclaim_list.hpp"

#include <boost/atomic.hpp>

#include <algorithm>
#include <stdexcept>
#include <cassert>

#include <windows.h>

/// \todo When thread local become available use it instead
/// of boost::thread_specific_ptr.

namespace tcl { namespace lock_free {

/// \brief Container of hazard pointers.
template<typename T, size_t Size>
class hazard_pointers
{
	struct context
	{
		context() : id_(0), ptr_(0) {}

		boost::atomic<unsigned> id_; 			//!< Thread that owns hazard pointer
		boost::atomic<const T*> ptr_;			//!< Pointer value

    private:
		context(const context&);
		context& operator=(const context&);
	};

	static const size_t delete_threashold_size = Size * 16;

	hazard_pointers(const hazard_pointers&);
	hazard_pointers& operator=(const hazard_pointers&);

public:
	hazard_pointers();
	~hazard_pointers();

	/// \brief Find free hazard pointer and allocate it for scope.
	class scoped_allocator
	{
	public:
		scoped_allocator(hazard_pointers& hps);

		scoped_allocator(scoped_allocator&&);
		scoped_allocator& operator=(scoped_allocator&&);

		~scoped_allocator();

		boost::atomic<const T*>& ref();

	private:
		scoped_allocator(const scoped_allocator&);
		scoped_allocator& operator=(const scoped_allocator&);

		context* ctx_;
	};

	bool outstanding_hp_for(const T* ptr) const;
	void reclaim_later(T* ptr);

private:
	context hps_[Size];
};

template<typename T, size_t Size>
hazard_pointers<T, Size>::scoped_allocator::scoped_allocator(hazard_pointers& hps)
{
	for(size_t i=0; i<Size; ++i)
	{
        context& ctx = hps.hps_[i];
		unsigned null_id = 0;

		if (ctx.id_.compare_exchange_strong(
				null_id, GetCurrentThreadId(), boost::memory_order_acq_rel, boost::memory_order_relaxed)
			)
		{
			ctx_ = &ctx;
			return;
		}
	}

	throw std::runtime_error("no free hazard pointers");
}

template<typename T, size_t Size>
hazard_pointers<T, Size>::scoped_allocator::~scoped_allocator()
{
	ctx_->ptr_.store(0, boost::memory_order_relaxed);
	ctx_->id_.store(0, boost::memory_order_release);
}

template<typename T, size_t Size>
boost::atomic<const T*>& hazard_pointers<T, Size>::scoped_allocator::ref()
{
	return ctx_->ptr_;
}

template<typename T, size_t Size>
hazard_pointers<T, Size>::hazard_pointers()
{
}

template<typename T, size_t Size>
hazard_pointers<T, Size>::~hazard_pointers()
{
	// Just assert that there are no hazard pointers in use
	for (size_t i=0; i<Size; ++i)
	{
        context& ctx = hps_[i];
		assert(
			"Ensure that there are no hazard pointers in use" &&
			0 == ctx.id_.load(boost::memory_order_relaxed));
	}
}

template<typename T, size_t Size>
bool hazard_pointers<T, Size>::outstanding_hp_for(const T* ptr) const
{
	for (size_t i=0; i<Size; ++i)
	{
        const context& ctx = hps_[i];
		if (0 != ctx.id_.load() && ctx.ptr_ == ptr)
			return true;
	}

	return false;
}

template<typename T, size_t Size>
void hazard_pointers<T, Size>::reclaim_later(T* ptr)
{
	auto& rl = get_reclaim_list();
	rl.push_back(hazard_pointers_reclaim_node(ptr));

	if (rl.size() < delete_threashold_size)
		return;

	auto b = rl.begin();
	auto e = rl.end();
	while (b != e)
	{
		if (outstanding_hp_for(b->ptr<T>()))
			++b;
		else
		{
			b->dispose();
			*b = std::move(*--e);
		}
	}

	rl.erase(e, rl.end());
}

}}
