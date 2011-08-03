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
template<typename T, size_t Size, typename Allocator>
class hazard_pointers : Allocator
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
	hazard_pointers(const Allocator& allocator);
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

    /// \brief Return allocator for freeing objects
    Allocator get_allocator();

private:
	context hps_[Size];
};

template<typename T, size_t Size, typename Allocator>
hazard_pointers<T, Size, Allocator>::scoped_allocator::scoped_allocator(hazard_pointers& hps)
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

template<typename T, size_t Size, typename Allocator>
hazard_pointers<T, Size, Allocator>::scoped_allocator::~scoped_allocator()
{
	ctx_->ptr_.store(0, boost::memory_order_relaxed);
	ctx_->id_.store(0, boost::memory_order_release);
}

template<typename T, size_t Size, typename Allocator>
boost::atomic<const T*>& hazard_pointers<T, Size, Allocator>::scoped_allocator::ref()
{
	return ctx_->ptr_;
}

template<typename T, size_t Size, typename Allocator>
hazard_pointers<T, Size, Allocator>::hazard_pointers(const Allocator& allocator)
    : Allocator(allocator)
{
}

template<typename T, size_t Size, typename Allocator>
hazard_pointers<T, Size, Allocator>::~hazard_pointers()
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

template<typename T, size_t Size, typename Allocator>
bool hazard_pointers<T, Size, Allocator>::outstanding_hp_for(const T* ptr) const
{
	for (size_t i=0; i<Size; ++i)
	{
        const context& ctx = hps_[i];
		if (0 != ctx.id_.load() && ctx.ptr_ == ptr)
			return true;
	}

	return false;
}

template<typename T, size_t Size, typename Allocator>
void hazard_pointers<T, Size, Allocator>::reclaim_later(T* ptr)
{
	auto& rl = get_reclaim_list();

    // Push ptr to reclaim list, use our allocator to deallocate object
    Allocator allocator(*this);

    auto deleter = 
        [allocator](void* ptr) -> void 
        {
            Allocator allocator1(allocator);  // CRAP!!!!
            allocator1.deallocate(static_cast<T*>(ptr), 1);
        };

	rl.push_back(
        hazard_pointers_reclaim_node(ptr, deleter) 
      );

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

template<typename T, size_t Size, typename Allocator>
Allocator hazard_pointers<T, Size, Allocator>::get_allocator()
{
    return *this;
}

}}
