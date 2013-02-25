#include <cassert>
#include <memory>
#include <boost/thread/thread.hpp>
#include "ftp/server.hpp"
#include "ftp/client.hpp"
#include "logs/logs.hpp"
#include "util/net/tlscontext.hpp"
#include "util/misc.hpp"

namespace ftp
{

namespace
{
void InterruptHandler(int /* signo */)
{
}

void InitialiseInterruption()
{
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = InterruptHandler;
  sigaction(SIGUSR1, &sa, nullptr);
}
}

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
      std::lock_guard<std::mutex> lock(queueMutex);
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
    instance.fds.resize(validIPs.size());
    int fdsIndex = 0;
    for (const auto& ip : validIPs)
    {
      ep = util::net::Endpoint(ip, port);
      std::unique_ptr<util::net::TCPListener> listener(new util::net::TCPListener(ep));
      int fd = listener->Socket();
      
      instance.fds[fdsIndex].fd = fd;
      instance.fds[fdsIndex].events = POLLIN;
      ++fdsIndex;
      
      instance.servers.insert(std::make_pair(fd, listener.release()));
      logs::Debug("Listening for clients on %1%", ep);
    }
  }
  catch (const util::net::NetworkError& e)
  {
    logs::Error("Unable to listen for clients on %1%: %2%", ep, e.Message());
    return false;
  }
  
  return true;
}

void Server::StopClients()
{
  logs::Debug("Stopping all connected clients..");
  for (auto& client : clients)
    client->Interrupt();
    
  for (auto& client : clients)
    client->Join();
    
  clients.clear();
}

void Server::AcceptClient(util::net::TCPListener& server)
{
  std::shared_ptr<ftp::Client> client(new ftp::Client());
  if (client->Accept(server)) 
  {
    client->Start();
    clients.insert(client);
  }
}

void Server::AcceptClients()
{
  sigset_t mask;
  sigfillset(&mask);
  sigdelset(&mask, SIGUSR1);

  for (auto& pfd : fds) pfd.revents = 0;
  
  struct timespec tv;
  tv.tv_sec = 0;
  tv.tv_nsec = 100000000;
  
  int n = ppoll(fds.data(), fds.size(), &tv, &mask);
  if (n < 0)
  {
    if (errno == EINTR)
    {
      HandleTasks();
    }
    else
    {
      logs::Error("Server select failed: %1%", util::Error::Failure(errno).Message());
      // ensure we don't poll rapidly on repeated select failures
      boost::this_thread::sleep(boost::posix_time::milliseconds(100));
    }
  }
  else
  {
    for (auto& pfd : fds)
    {
      if (pfd.revents & POLLIN)
        AcceptClient(*servers.at(pfd.fd));
    }
  }
}

void Server::Run()
{
  InitialiseInterruption();
  util::SetProcessTitle("SERVER");
  while (!isShutdown)
  {
    AcceptClients();
  }
  
  StopClients();
}

void Server::InnerSetShutdown()
{
  isShutdown = true;
  pthread_kill(thread.native_handle(), SIGUSR1);
}

void Server::PushTask(const TaskPtr& task)
{
  { 
    std::lock_guard<std::mutex> lock(instance.queueMutex); 
    instance.queue.push(task);
  }
  
  pthread_kill(instance.thread.native_handle(), SIGUSR1);
}

void Server::CleanupClient(const std::shared_ptr<Client>& client)
{
  assert(client->State() == ClientState::Finished);
  client->Join();
  clients.erase(client);
  logs::Debug("Client finished");
}

} // end ftp namespace
