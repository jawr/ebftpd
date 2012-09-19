#include <unistd.h>
#include <exception>
#include <boost/thread/thread.hpp>
#include "util/thread.hpp"
namespace util
{

void Thread::Start()
{ 
  thread = boost::thread(&Thread::Main, this);  
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

void Thread::Stop()
{
  thread.interrupt();
  write(pipe[1], "1", 1);
  thread.join();
}

void Thread::Join()
{
  thread.join();
}

// end util namespace
}
