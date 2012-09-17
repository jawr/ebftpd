#include <exception>
#include <boost/thread/thread.hpp>
#include "util/thread.hpp"
namespace util
{

void Thread::Start()
{ 
  try
  {
    Run();
  }
  catch (boost::thread_interrupted& interruption)
  {
    // interrupted, expected
  }
  catch (std::exception& e)
  {
    // handle?
  }
}

void Thread::Join()
{
  thread->join();
}

// end util namespace
}
