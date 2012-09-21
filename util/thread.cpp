#include <unistd.h>
#include <exception>
#include <boost/thread/thread.hpp>
#include "util/thread.hpp"
#include "util/error.hpp"

namespace util
{

ThreadSelect::ThreadSelect()
{
  if (pipe(interruptPipe) < 0) throw SystemError(errno);
}


ThreadSelect::~ThreadSelect()
{
  if (interruptPipe[0] >= 0) close(interruptPipe[0]);
  if (interruptPipe[1] >= 0) close(interruptPipe[1]);
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
  write(interruptPipe[1], "1", 1);
  thread.join();
}

void Thread::Join()
{
  thread.join();
}

// end util namespace
}
