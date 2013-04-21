#ifndef __UTIL_SPINMUTEX_HPP
#define __UTIL_SPINMUTEX_HPP

#include <atomic>
#include <cassert>

namespace util
{

class SpinMutex
{
	std::atomic_flag flag;
	
public:
	SpinMutex() : flag(ATOMIC_FLAG_INIT) { }
	
	~SpinMutex()
	{
		assert(!flag.test_and_set(std::memory_order_acquire));
	}
	
	void lock()
	{
		while (flag.test_and_set(std::memory_order_acquire))
			;
	}
	
	bool try_lock()
	{
		return !flag.test_and_set(std::memory_order_acquire);
	}
	
	void unlock()
	{
		 flag.clear(std::memory_order_release); 
	}
};

} /* util namespace */

#endif
