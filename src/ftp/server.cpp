#include <cassert>
#include <memory>
#include <boost/thread/thread.hpp>
#include <boost/thread/locks.hpp>
#include <sys/select.h>
#include "ftp/server.hpp"
#include "ftp/client.hpp"
#include "logs/logs.hpp"
#include "util/net/tlscontext.hpp"

namespace ftp
{

Server Server::instance;

void Server::StartThread()
{
  logs::Debug("Starting listener thread..");
  instance.Start();
}

void Server::JoinThread()
{
  instance.Join();
}

void Server::SetShutdown()
{   
  logs::Debug("Stopping listener thread..");
  instance.InnerSetShutdown();
}

void Server::HandleTasks()
{
  //logs::Debug("Handling listener tasks..");
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

bool Server::Initialise(const std::vector<std::string>& validIPs, int32_t port)
{
  assert(!validIPs.empty());
  util::net::Endpoint ep;
  try
  {
    for (const auto& ip : validIPs)
    {
      ep = util::net::Endpoint(ip, port);
      instance.servers.push_back(new util::net::TCPListener(ep));
      logs::Debug("Listening for clients on %1%", ep);
    }
  }
  catch (const util::net::NetworkError& e)
  {
    logs::Error("Unable to listen for clients on %1% : %2%", ep, e.Message());
    return false;
  }
  
  return true;
}

void Server::HandleClients()
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
        logs::Debug("Client finished");
      }
      else
        ++it;
      // we should probably log if a client hasnt been joinable for too long
    }
    else 
      ++it;
  }
}

void Server::StopClients()
{
  logs::Debug("Stopping all connected clients..");
  for (auto& client : clients)
    client.Interrupt();
    
  for (auto& client : clients)
    client.Join();
    
  clients.clear();
}

void Server::AcceptClient(util::net::TCPListener& server)
{
  std::unique_ptr<ftp::Client> client(new ftp::Client());
  if (client->Accept(server)) 
  {
    client->Start();
    boost::lock_guard<boost::mutex> lock(clientMtx);
    clients.push_back(client.release());
  }
}

void Server::AcceptClients()
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
  
  int n = select(max + 1, &readSet, nullptr, nullptr, &tv);
  if (n < 0)
  {
    logs::Error("Server select failed: %1%", util::Error::Failure(errno).Message());
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

void Server::Run()
{
  while (!isShutdown)
  {
    AcceptClients();
    HandleClients();
  }
  
  StopClients();
}

} // end ftp namespace
