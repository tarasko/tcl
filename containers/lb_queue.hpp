#pragma

#include <boost/thread/mutex.hpp>
#include <queue>

namespace tcl { namespace containers {

template<typename T>
class lb_queue
{
public:
	void push(const T& val)
	{
		boost::lock_guard<boost::mutex> g(guard_);
		queue_.push(val);
	}

	bool try_pop(T& result)
	{
		boost::lock_guard<boost::mutex> g(guard_);
		if (queue_.empty())
			return false;

		result = std::move(queue_.front());
		queue_.pop();
		return true;
	}

private:
	std::queue<T> queue_;
	boost::mutex guard_;
};

}}
