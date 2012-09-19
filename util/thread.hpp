#ifndef __UTIL_THREAD_HPP
#define __UTIL_THREAD_HPP
#include <boost/thread/thread.hpp>
namespace util
{
class Thread
{
  boost::thread thread;
  int pipe[2];

  void Main();
public:
  Thread() : pipe(), thread() {};
  virtual ~Thread() {};
  void Start();
  void Stop();
  void Join();
  virtual void Run() {};
};
// end util namespace
}
#endif
