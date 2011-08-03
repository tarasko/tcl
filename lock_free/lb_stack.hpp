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
		boost::lock_guard<boost::mutex> g(guard_);
		stack_.push(val);
	}

	bool try_pop(T& result)
	{
		boost::lock_guard<boost::mutex> g(guard_);
		if (stack_.empty())
			return false;

		result = std::move(stack_.top());
		stack_.pop();
		return true;
	}

private:
	std::stack<T> stack_;
	boost::mutex guard_;
};

}}
