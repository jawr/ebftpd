#ifndef __UTIL_THREAD_HPP
#define __UTIL_THREAD_HPP

#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>

namespace util
{

class Thread
{
  void Main();

protected:
  boost::thread thread;
  bool started;
  
  virtual void Run() = 0;

public:
  Thread() : started(false) { }
  virtual ~Thread() { }

  void Start();
  void Join();
  bool TryJoin();
  void Stop(bool join = false);
  bool Started() const { return started; }
};

// end util namespace
}

#endif
