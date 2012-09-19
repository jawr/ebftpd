#ifndef __UTIL_THREAD_HPP
#define __UTIL_THREAD_HPP
#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
namespace util
{
class Thread
{
  boost::thread thread;
  int pipe[2];
protected:
  boost::condition cond;
  boost::mutex mtx;

  void Main();
public:
  Thread() :  cond(), mtx(), pipe(), thread() {};
  virtual ~Thread() {};
  void Start();
  void Stop();
  void Join();
  virtual void Run() {};
};
// end util namespace
}
#endif
