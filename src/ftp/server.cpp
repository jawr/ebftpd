//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <csignal>
#include <cassert>
#include <memory>
#include <boost/thread/thread.hpp>
#include <poll.h>
#include "ftp/server.hpp"
#include "ftp/client.hpp"
#include "logs/logs.hpp"
#include "util/net/tlscontext.hpp"
#include "util/misc.hpp"

namespace ftp
{

std::unique_ptr<Server> Server::instance;
boost::once_flag Server::instanceOnce = BOOST_ONCE_INIT;

Server::Server() :
  shutdown(false)
{
}

void Server::Listen(const std::vector<std::string>& validIPs, int port)
{
  assert(!validIPs.empty());
  util::net::Endpoint ep;
  try
  {
    fds.resize(validIPs.size() + 1);
    int fdsIndex = 0;
    for (const auto& ip : validIPs)
    {
      ep = util::net::Endpoint(ip, port);
      std::unique_ptr<util::net::TCPListener> listener(new util::net::TCPListener(ep));
      int fd = listener->Socket();
      
      fds[fdsIndex].fd = fd;
      fds[fdsIndex].events = POLLIN;
      ++fdsIndex;
      
      servers.insert(fd, listener.release());
      logs::Debug("Listening for clients on %1%", ep);
    }
    
    fds[fdsIndex].fd = interruptPipe.ReadFd();
    fds[fdsIndex].events = POLLIN;
  }
  catch (const util::net::NetworkError& e)
  {
    logs::Error("Unable to listen for clients on %1%: %2%", ep, e.Message());
    throw e;
  }
}

void Server::StartThread()
{
  logs::Debug("Starting listener thread..");
  Start();
}

void Server::JoinThread()
{
  Join();
}

void Server::Cleanup()
{
  instance = nullptr;
}

void Server::HandleTasks()
{
  //logs::Debug("Handling listener tasks..");
  TaskPtr task;
  while (true)
  {
    {
      std::lock_guard<std::mutex> lock(queueMutex);
      if (queue.empty()) break;
      task = queue.front();
      queue.pop();
    }

    task->Execute(*this);
  }
}

bool Server::Initialise(const std::vector<std::string>& validIPs, int port)
{
  try
  {
    Get().Listen(validIPs, port);
  }
  catch (const util::net::NetworkError&)
  {
    return false;
  }
  
  return true;
}

void Server::CreateInstance()
{
  instance.reset(new Server());
}

Server& Server::Get()
{
  boost::call_once(&CreateInstance, instanceOnce);
  return *instance;
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
    clients.insert(client.release());
  }
}

void Server::AcceptClients()
{
  sigset_t mask;
  sigfillset(&mask);
  sigdelset(&mask, SIGUSR1);

  for (auto& pfd : fds) pfd.revents = 0;
  
  int n = poll(fds.data(), fds.size(), 100);
  if (n < 0)
  {
    logs::Error("Server select failed: %1%", util::Error::Failure(errno).Message());
    // ensure we don't poll rapidly on repeated select failures
    boost::this_thread::sleep(boost::posix_time::milliseconds(100));
  }
  else
  {
    // last pullfd is interrupt pipe
    if (fds.back().revents & POLLIN)
    {
      interruptPipe.Acknowledge();
      HandleTasks();
    }
    
    for (auto it = fds.begin(); it != fds.end() - 1; ++it)
    {
      if (it->revents & POLLIN)
        AcceptClient(servers.at(it->fd));
    }
  }
}

void Server::Run()
{
  util::SetProcessTitle("SERVER");
  logs::SetThreadIDPrefix('L' /* listener/server */);
  while (!shutdown)
  {
    AcceptClients();
  }
  
  StopClients();
}

void Server::Shutdown()
{
  logs::Debug("Stopping listener thread..");
  shutdown = true;
  interruptPipe.Interrupt();
}

void Server::PushTask(const TaskPtr& task)
{
  { 
    std::lock_guard<std::mutex> lock(queueMutex); 
    queue.push(task);
  }
  
  interruptPipe.Interrupt();
}

void Server::CleanupClient(Client& client)
{
  assert(client.State() == ClientState::Finished);
  client.Join();
  clients.erase(client);
  logs::Debug("Client finished");
}

} // end ftp namespace
