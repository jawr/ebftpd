#ifndef __FTP_SERVER_HPP
#define __FTP_SERVER_HPP

#include <ostream>
#include <vector>
#include <unordered_set>
#include <queue>
#include <atomic>
#include <mutex>
#include <memory>
#include <poll.h>
#include <boost/ptr_container/ptr_list.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include "ftp/task/types.hpp"
#include "ftp/task/task.hpp"
#include "util/thread.hpp"
#include "util/net/tcplistener.hpp"
#include "util/net/tcpsocket.hpp"
#include "util/interruptpipe.hpp"

namespace ftp
{

class Client;

class Server : public util::Thread
{
  std::unordered_map<int, std::shared_ptr<util::net::TCPListener>> servers;
  std::vector<struct pollfd> fds;
  std::vector<std::string> validIPs;
  int32_t port;

  std::unordered_set<std::shared_ptr<Client>> clients;

  std::mutex queueMutex;
  std::queue<TaskPtr> queue;
  
  std::atomic_bool isShutdown;
  
  void AcceptClients();
  void AcceptClient(util::net::TCPListener& server);

  void Run();
  void HandleTasks();
  void StopClients();
  void CleanupClient(const std::shared_ptr<Client>& client);

  void InnerSetShutdown();
  
  Server() : port(-1), isShutdown(false) { }

  static void PushTask(const TaskPtr& task);  
  static Server instance;
  
public:
  static bool Initialise(const std::vector<std::string>& validIPs, int32_t port);
  
  static void StartThread();
  static void JoinThread();
  static void SetShutdown();

  friend class task::KickUser;
  friend class task::GetOnlineUsers;
  friend class task::LoginKickUser;
  friend class task::UserUpdate;
  friend class task::Task;
  friend class task::OnlineUserCount;
  friend class task::ClientFinished;
  
  friend void SignalHandler(int);
};

}

#endif
