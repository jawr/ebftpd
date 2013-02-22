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
