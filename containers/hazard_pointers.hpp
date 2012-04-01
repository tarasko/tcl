#pragma once

#include "hazard_pointers_reclaim_list.hpp"

#include <boost/atomic.hpp>

#include <stdexcept>
#include <cassert>

#include <windows.h>

namespace tcl { namespace containers {

/// \brief Container of hazard pointers.
///
/// More on hazard pointers http://drdobbs.com/cpp/184401890
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

	static const size_t delete_threashold_size = Size * 2;

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

    /// \brief Return true if this pointer is marked as hazard by another thread
	bool outstanding_hp_for(const T* ptr) const;

    /// \brief Push pointer together with allocater to reclaim later list.
    /// When reclaim list grows to some threashold size, it will be searched for 
    /// pointers that are ready to delete and delete them using allocator.deallocate
    /// method
    template<typename Allocator>
	void reclaim_later(T* ptr, Allocator& allocator);

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
template<typename Allocator>
void hazard_pointers<T, Size>::reclaim_later(T* ptr, Allocator& allocator)
{
	auto& rl = get_reclaim_list();

	rl.push_back(
        hazard_pointers_reclaim_node(
            ptr
          , [allocator](void* ptr) -> void 
            {   
                const_cast<Allocator&>(allocator).deallocate(static_cast<T*>(ptr), 1);
            }
          ) 
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

}}
