#ifndef __FTP_SERVER_HPP
#define __FTP_SERVER_HPP

#include <ostream>
#include <vector>
#include <unordered_set>
#include <queue>
#include <atomic>
#include <mutex>
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
  typedef boost::ptr_list<Client> ClientList;

  boost::ptr_vector<util::net::TCPListener> servers;
  std::vector<std::string> validIPs;
  int32_t port;
  util::InterruptPipe interruptPipe;

  std::mutex clientMtx;
  boost::ptr_list<Client> clients;

  std::queue<TaskPtr> queue;
  std::mutex taskMtx;
  
  std::atomic_bool isShutdown;
  
  void AcceptClients();
  void AcceptClient(util::net::TCPListener& server);
  void HandleClients();
  void Run();
  void HandleTasks();
  void StopClients();

  void InnerSetShutdown()
  {
    isShutdown = true;
    instance.interruptPipe.Interrupt();
  }

  Server() : port(-1), isShutdown(false) { }

  static void PushTask(const TaskPtr& task)
  {
    { 
      std::lock_guard<std::mutex> lock(instance.taskMtx); 
      instance.queue.push(task);
    }
    instance.interruptPipe.Interrupt();
  }
  
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
  
  friend void SignalHandler(int);
};

}

#endif
