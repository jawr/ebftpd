#include <boost/thread.hpp>
#include "ftp/listener.hpp"
namespace ftp
{
Listener::Listener() : addr("localhost", 21)
{
  boost::thread(&Listener::Main, this);
}

Listener::Listener(const std::string& ip, uint16_t) : addr(ip, port)
{
  boost::thread(&Listener::Main, this);
}
  
Listener::~Listener()
{
  thread.interrupt();
  thread.join();
}

void Listener::Main()
{
  try
  {
    Listen();
  }
  catch(boost::thread_interrupted& interruption)
  {
    // interrupt, expected
  }
  catch (std::exception& e)
  {
    // handle
  }
}

void Listener::Listen()
{

}
  
}
// end ftp namespace
}
