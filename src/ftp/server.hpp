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

#ifndef __FTP_SERVER_HPP
#define __FTP_SERVER_HPP

#include <cstdint>
#include <functional>
#include <vector>
#include <ostream>
#include <queue>
#include <atomic>
#include <mutex>
#include <memory>
#include <poll.h>
#include <boost/ptr_container/ptr_unordered_map.hpp>
#include <boost/ptr_container/ptr_unordered_set.hpp>
#include <boost/thread/once.hpp>
#include "ftp/task/types.hpp"
#include "ftp/task/task.hpp"
#include "util/thread.hpp"
#include "util/net/tcplistener.hpp"
#include "util/net/tcpsocket.hpp"
#include "util/interruptpipe.hpp"

namespace std
{
  template <>
  struct hash<ftp::Client> : public unary_function<ftp::Client, size_t>
  {
  private:
    std::hash<intptr_t> hash;
    
  public:
    size_t operator()(const ftp::Client& client) const
    {
      return hash(reinterpret_cast<intptr_t>(&client));
    }
  };

  template <>
  class equal_to<ftp::Client> : public binary_function<ftp::Client, ftp::Client, bool>
  {
    std::equal_to<intptr_t> equal_to;

  public:
    size_t operator()(const ftp::Client& client1, const ftp::Client& client2) const
    {
      return equal_to(reinterpret_cast<intptr_t>(&client1), reinterpret_cast<intptr_t>(&client2));
    }
  };
}

namespace ftp
{

class Client;

class Server : public util::Thread
{
  boost::ptr_unordered_map<int, util::net::TCPListener> servers;
  std::vector<struct pollfd> fds;
  util::InterruptPipe interruptPipe;

  boost::ptr_unordered_set<Client, std::hash<Client>, std::equal_to<Client>> clients;

  std::mutex queueMutex;
  std::queue<TaskPtr> queue;
  
  std::atomic_bool shutdown;
  
  Server();

  void Listen(const std::vector<std::string>& validIPs, int port);
  void AcceptClients();
  void AcceptClient(util::net::TCPListener& server);

  void Run();
  void HandleTasks();
  void StopClients();
  void CleanupClient(Client& client);
  void PushTask(const TaskPtr& task);  
  
  static std::unique_ptr<Server> instance;
  static boost::once_flag instanceOnce;
  
  static void CreateInstance();
  
public:
  static bool Initialise(const std::vector<std::string>& validIPs, int port);
  static void Cleanup();
  static Server& Get();
  
  void StartThread();
  void JoinThread();
  void Shutdown();

  friend class task::KickUser;
  friend class task::LoginKickUser;
  friend class task::UserUpdate;
  friend class task::Task;
  friend class task::ClientFinished;
  
  friend void SignalHandler(int);
};

}

#endif
