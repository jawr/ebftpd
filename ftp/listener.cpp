#include <memory>
#include <boost/thread/thread.hpp>
#include <boost/thread/locks.hpp>
#include <sys/select.h>
#include "ftp/listener.hpp"
#include "ftp/client.hpp"
#include "logger/logger.hpp"
#include "util/exception.hpp"

namespace ftp
{

bool Listener::Initialise()
{
  try
  {
    server.listen(addr);
  }
  catch (const util::network_error& e)
  {
    logger::error << "Unable to listen for clients on " << addr << ": " << e.what() << logger::endl;
    return false;
  }
  
  logger::ftpd << "Listening for clients on " << addr << logger::endl;
  return true;
}

void Listener::HandleClients()
{
  // erase any clients that are done
  for (ClientList::iterator it = clients.begin();
       it != clients.end();)
  {
    ftp::Client& client = *it;
    if (client.Finished())
    {
      client.Join();
      clients.erase(it++);
      logger::ftpd << "Client finished" << logger::endl;
    }
    else ++it;
  }
}

void Listener::AcceptClient()
{
  std::auto_ptr<ftp::Client> client(new ftp::Client());
  if (client->Accept(server)) 
  {
    client->Start();
    clients.push_back(client.release());
  }
}

void Listener::AcceptClients()
{
  fd_set readSet;
  FD_ZERO(&readSet);
  FD_SET(server.socket(), &readSet);
  FD_SET(interruptPipe[0], &readSet);
  int max = std::max(server.socket(), interruptPipe[0]);
  
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 100000;
  
  int n = select(max + 1, &readSet, NULL, NULL, &tv);
  if (n < 0)
  {
    logger::error << "Listener select failed: " << util::Error::Failure(errno).Message() << logger::endl;
    // ensure we don't poll rapidly on repeated select failures
    boost::this_thread::sleep(boost::posix_time::milliseconds(100));
  }
  else if (FD_ISSET(server.socket(), &readSet)) AcceptClient();
  else if (FD_ISSET(interruptPipe[0], &readSet))
  {
    boost::this_thread::interruption_point();
    // possibly use this to refresh our config too ??
    // if port has changed in config, we have to renew our server object with
    // new endpoint
  }
}

void Listener::Run()
{
  while (true)
  {
    AcceptClients();
    HandleClients();
  }
}

}
// end ftp namespace

#ifdef FTP_LISTENER_TEST

#include <iostream>

int main()
{
  ftp::Listener l("127.0.0.1", 1234);
  if (!l.Initialise()) return 1;
  l.Start();
  l.Join();
  return 0;
}

#endif
