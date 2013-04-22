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

#ifndef __SEMAPHORE_HPP
#define __SEMAPHORE_HPP

#include <condition_variable>
#include <mutex>
#include <boost/noncopyable.hpp>

namespace util
{
  class semaphore : boost::noncopyable
  {
  private:
    unsigned int count;
    std::mutex mutex;
    std::condition_variable cond;
    
  public:
    explicit semaphore(unsigned int count) :
      count(count),
      mutex(),
      cond()
    {
    }
    
    void signal()
    {
      std::lock_guard<std::mutex> lock(mutex);
      ++count;
      cond.notify_one();
    }
    
    void wait()
    {
      std::unique_lock<std::mutex> lock(mutex);
      while (!count) cond.wait(lock);
      --count;
    }
  };
};

#endif
