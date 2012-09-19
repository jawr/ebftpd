#include <boost/thread.hpp>
#include "ftp/listener.hpp"
#include <iostream>
namespace ftp
{
void Listener::Run()
{
  std::cout << "Run" << std::endl;
}
  
}
// end ftp namespace

#ifdef LISTENER_TEST
int main()
{
  ftp::Listener l;
  l.Start();
  l.Join();
  return 0;
}
#endif
