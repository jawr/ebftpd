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
  void Main();
public:
  Thread() : thread() {};
  virtual ~Thread() {};
  void Start();
  void Join();
  virtual void Run() = 0;
  virtual void Stop();
};

class ThreadConsumer : public Thread
{
protected:
  boost::condition cond;
  boost::mutex mtx;

public:
  ThreadConsumer() :  cond(), mtx() {};
  virtual ~ThreadConsumer() {};
  void Stop();
};

class ThreadSelect : public Thread
{
protected:
  int pipe[2];
public:
  ThreadSelect() { pipe[0] = -1; pipe[1] = -1; };
  virtual ~ThreadSelect() {};
  void Stop();
// end util namespace
}
#endif
