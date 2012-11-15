#include <cassert>
#include <memory>
#include <boost/thread/thread.hpp>
#include <boost/thread/locks.hpp>
#include <sys/select.h>
#include "ftp/listener.hpp"
#include "ftp/client.hpp"
#include "logs/logs.hpp"
#include "util/net/tlscontext.hpp"

namespace ftp
{

Listener Listener::instance;

void Listener::StartThread()
{
  logs::debug << "Starting listener thread.." << logs::endl;
  instance.Start();
}

void Listener::JoinThread()
{
  instance.Join();
}

void Listener::SetShutdown()
{   
  logs::debug << "Stopping listener thread.." << logs::endl;
  instance.InnerSetShutdown();
}

void Listener::HandleTasks()
{
  //logs::debug << "Handling listener tasks.." << logs::endl;
  TaskPtr task;
  while (true)
  {
    {
      boost::lock_guard<boost::mutex> lock(taskMtx);
      if (queue.empty()) break;
      task = queue.front();
      queue.pop();
    }

    task->Execute(*this);
  }
}

bool Listener::Initialise(const std::vector<std::string>& validIPs, int32_t port)
{
  assert(!validIPs.empty());
  util::net::Endpoint ep;
  try
  {
    for (const auto& ip : validIPs)
    {
      ep = util::net::Endpoint(ip, port);
      instance.servers.push_back(new util::net::TCPListener(ep));
      logs::debug << "Listening for clients on " << ep << logs::endl;
    }
  }
  catch (const util::net::NetworkError& e)
  {
    logs::error << "Unable to listen for clients on " << ep
                  << ": " << e.Message() << logs::endl;
    return false;
  }
  
  return true;
}

void Listener::HandleClients()
{
  boost::lock_guard<boost::mutex> lock(clientMtx);
  for (ClientList::iterator it = clients.begin();
       it != clients.end();)
  {
    ftp::Client& client = *it;
    if (client.State() == ClientState::Finished)
    {
      if (client.TryJoin())
      {
        clients.erase(it++);
        logs::debug << "Client finished" << logs::endl;
      }
      else
        ++it;
      // we should probably log if a client hasnt been joinable for too long
    }
    else 
      ++it;
  }
}

void Listener::StopClients()
{
  logs::debug << "Stopping all connected clients.." << logs::endl;
  for (auto& client : clients)
    client.Interrupt();
    
  for (auto& client : clients)
    client.Join();
    
  clients.clear();
}

void Listener::AcceptClient(util::net::TCPListener& server)
{
  std::unique_ptr<ftp::Client> client(new ftp::Client());
  if (client->Accept(server)) 
  {
    client->Start();
    boost::lock_guard<boost::mutex> lock(clientMtx);
    clients.push_back(client.release());
  }
}

void Listener::AcceptClients()
{
  fd_set readSet;
  FD_ZERO(&readSet);

  FD_SET(interruptPipe.ReadFd(), &readSet);
  int max = interruptPipe.ReadFd();
  
  for (auto& server : servers)
  {
    FD_SET(server.Socket(), &readSet);
    max = std::max(server.Socket(), interruptPipe.ReadFd());
  }
  
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 100000;
  
  int n = select(max + 1, &readSet, NULL, NULL, &tv);
  if (n < 0)
  {
    logs::error << "Listener select failed: " << util::Error::Failure(errno).Message() << logs::endl;
    // ensure we don't poll rapidly on repeated select failures
    boost::this_thread::sleep(boost::posix_time::milliseconds(100));
  }
  else if (FD_ISSET(interruptPipe.ReadFd(), &readSet))
  {
    char ch;
    (void) read(interruptPipe.ReadFd(), &ch, sizeof(ch));
    HandleTasks();
  }
  else
  {
    for (auto& server : servers)
      if (FD_ISSET(server.Socket(), &readSet)) AcceptClient(server);
  }
}

void Listener::Run()
{
  while (!isShutdown)
  {
    AcceptClients();
    HandleClients();
  }
}

} // end ftp namespace

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
