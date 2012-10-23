#ifndef __FTP_LISTENER_HPP
#define __FTP_LISTENER_HPP

#include <ostream>
#include <vector>
#include <unordered_set>
#include <queue>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/ptr_container/ptr_list.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include "ftp/client.hpp"
#include "ftp/task/types.hpp"
#include "ftp/task/task.hpp"
#include "util/thread.hpp"
#include "util/net/tcplistener.hpp"
#include "util/net/tcpsocket.hpp"
#include "util/pipe.hpp"

namespace ftp
{

class Listener : public util::Thread
{
  typedef boost::ptr_list<Client> ClientList;

  boost::ptr_vector<util::net::TCPListener> servers;
  boost::ptr_list<Client> clients;
  std::vector<std::string> validIPs;
  int32_t port;
  util::Pipe interruptPipe;

  std::queue<TaskPtr> queue;
  
  boost::mutex taskMtx;
  boost::mutex clientMtx;
  
  void AcceptClients();
  void AcceptClient(util::net::TCPListener& server);
  void HandleClients();
  void Run();
  void RunTask();

  static Listener instance;
  
public:
  Listener() : port(-1) { }
  static bool Initialise(const std::vector<std::string>& validIPs, int32_t port);
  
  static void Stop();

  static void PushTask( const TaskPtr& task)
  {
    { 
      boost::lock_guard<boost::mutex> lock(instance.taskMtx); 
      instance.queue.push(task);
    }
    instance.interruptPipe.Interrupt();
  }

  static void StartThread();
  static void StopThread();
  static void JoinThread() { instance.Join(); }
    

  // tasks
  friend task::Task;
  friend task::KickUser;
};

}

#endif
