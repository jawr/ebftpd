#ifndef __SEMAPHORE_HPP
#define __SEMAPHORE_HPP

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/noncopyable.hpp>

namespace util
{
  class semaphore : boost::noncopyable
  {
  private:
    unsigned int count;
    boost::mutex mutex;
    boost::condition_variable cond;
    
  public:
    explicit semaphore(unsigned int count) :
      count(count),
      mutex(),
      cond()
    {
    }
    
    void signal()
    {
      boost::lock_guard<boost::mutex> lock(mutex);
      ++count;
      cond.notify_one();
    }
    
    void wait()
    {
      boost::unique_lock<boost::mutex> lock(mutex);
      while (!count) cond.wait(lock);
      --count;
    }
  };
};

#endif
