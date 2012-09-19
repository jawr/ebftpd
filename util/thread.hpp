#ifndef __UTIL_THREAD_HPP
#define __UTIL_THREAD_HPP
#include <boost/thread/thread.hpp>
namespace util
{
class Thread
{
  boost::thread thread;
  int[2] pipe;

  void Main();
public:
  Thread() {};
  virtual ~Thread();
  void Start();
  void Stop();
  void Join();
  virtual void Run() {};
};
// end util namespace
}
#endif
