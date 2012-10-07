#include <cassert>
#include <memory>
#include <boost/thread/thread.hpp>
#include <boost/thread/locks.hpp>
#include <sys/select.h>
#include "ftp/listener.hpp"
#include "ftp/client.hpp"
#include "logger/logger.hpp"
#include "util/net/tlscontext.hpp"

namespace ftp
{

bool Listener::Initialise(const std::vector<std::string>& validIPs, int32_t port)
{
  assert(!validIPs.empty());
  util::net::Endpoint ep;
  try
  {
    for (const auto& ip : validIPs)
    {
      ep = util::net::Endpoint(ip, port);
      servers.push_back(new util::net::TCPListener(ep));
      logger::ftpd << "Listening for clients on " << ep << logger::endl;
    }
  }
  catch (const util::net::NetworkError& e)
  {
    logger::error << "Unable to listen for clients on " << ep
                  << ": " << e.Message() << logger::endl;
    return false;
  }
  
  return true;
}

void Listener::HandleClients()
{
  // erase any clients that are done
  for (ClientList::iterator it = clients.begin();
       it != clients.end();)
  {
    ftp::Client& client = *it;
    if (client.IsFinished())
    {
      client.Join();
      clients.erase(it++);
      logger::ftpd << "Client finished" << logger::endl;
    }
    else ++it;
  }
}

void Listener::AcceptClient(util::net::TCPListener& server)
{
  std::unique_ptr<ftp::Client> client(new ftp::Client());
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

  FD_SET(interruptPipe[0], &readSet);
  int max = interruptPipe[0];
  
  for (auto& server : servers)
  {
    FD_SET(server.Socket(), &readSet);
    max = std::max(server.Socket(), interruptPipe[0]);
  }
  
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
  else if (FD_ISSET(interruptPipe[0], &readSet))
  {
    boost::this_thread::interruption_point();
    // possibly use this to refresh our config too ??
    // if port has changed in config, we have to renew our server object with
    // new endpoint
  }
  else
  {
    for (auto& server : servers)
      if (FD_ISSET(server.Socket(), &readSet)) AcceptClient(server);
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
  util::net::TLSServerContext::Initialise("server.pem");
  ftp::Listener l("127.0.0.1", 1234);
  if (!l.Initialise()) return 1;
  l.Start();
  l.Join();
  return 0;
}

#endif
