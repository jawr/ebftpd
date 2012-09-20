#include <unistd.h>
#include <exception>
#include <boost/thread/thread.hpp>
#include "util/thread.hpp"
namespace util
{

ThreadSelect::~ThreadSelect()
{
  if (pipe[0] >= 0) close(pipe[0]);
  if (pipe[1] >= 0) close(pipe[1]);
}

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

void ThreadConsumer::Stop()
{
  thread.interrupt();
  thread.join();
}

void ThreadSelect::Stop()
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
