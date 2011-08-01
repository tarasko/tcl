#pragma

#include <boost/thread/mutex.hpp>
#include <stack>

namespace tcl { namespace lock_free {

template<typename T>
class lb_stack
{
public:
	void push(const T& val)
	{
		std::shared_ptr<T> val_ptr(new T(val));
		boost::lock_guard<boost::mutex> g(guard_);
		stack_.push(val_ptr);
	}

	std::shared_ptr<T> try_pop()
	{
		boost::lock_guard<boost::mutex> g(guard_);
		if (stack_.empty())
			return std::shared_ptr<T>();

		auto ret = std::move(stack_.top());
		stack_.pop();
		return ret;
	}

private:
	std::stack<std::shared_ptr<T>> stack_;
	boost::mutex guard_;
};

}}
