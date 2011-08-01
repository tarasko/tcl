#pragma

#include <boost/thread/mutex.hpp>
#include <queue>

namespace tcl { namespace lock_free {

template<typename T>
class lb_queue
{
public:
	void push(const T& val)
	{
		std::shared_ptr<T> val_ptr = std::make_shared<T>(val);
		boost::lock_guard<boost::mutex> g(guard_);
		queue_.push(val_ptr);
	}

	std::shared_ptr<T> try_pop()
	{
		boost::lock_guard<boost::mutex> g(guard_);
		if (queue_.empty())
			return std::shared_ptr<T>();

		auto ret = std::move(queue_.front());
		queue_.pop();
		return ret;
	}

private:
	std::queue<std::shared_ptr<T>> queue_;
	boost::mutex guard_;
};

}}
