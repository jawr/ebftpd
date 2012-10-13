#include <unistd.h>
#include <exception>
#include <boost/thread/thread.hpp>
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
