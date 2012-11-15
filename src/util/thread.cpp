#include <unistd.h>
#include <exception>
#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "util/thread.hpp"
#include "util/error.hpp"

namespace util
{

void Thread::Start()
{ 
  assert(!started);
  thread = boost::thread(&Thread::Main, this);  
  started = true;
}

void Thread::Stop(bool join)
{
  if (started)
  {
    thread.interrupt();
    if (join)
    {
      thread.join();
      started = false;
    }
  }
}

void Thread::Join()
{
  if (started) 
  {
    thread.join();
    started = false;
  }
}

bool Thread::TryJoin()
{
  if (!started) return true;
  if (thread.timed_join(boost::posix_time::seconds(0)))
  {
    started = false;
    return true;
  }
  return false;
}

void Thread::Main()
{
  try
  {
    Run();
  }
  catch (boost::thread_interrupted& interruption)
  {
    // interrupted, expected
  }
}

// end util namespace
}
