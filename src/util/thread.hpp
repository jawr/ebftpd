#ifndef __UTIL_THREAD_HPP
#define __UTIL_THREAD_HPP

#include <boost/thread/thread.hpp>

namespace util
{

class Thread
{
  inline void Main()
  {
    try
    {
      Run();
    }
    catch (const boost::thread_interrupted&)
    {
    }
  }

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
