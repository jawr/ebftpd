//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
