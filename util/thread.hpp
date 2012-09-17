#ifndef __UTIL_THREAD_HPP
#define __UTIL_THREAD_HPP
#include <boost/thread/thread.hpp>
namespace util
{
class Thread
{
  boost::thread *thread;
public:
  Thread() { thread = new boost::thread(&Thread::Start, this); };
  ~Thread() { thread->interrupt(); thread->join(); };
  void Start();
  void Join();
  virtual void Run() {};
};
// end util namespace
}
#endif
