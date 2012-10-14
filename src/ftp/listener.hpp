#ifndef __FTP_LISTENER_HPP
#define __FTP_LISTENER_HPP

#include <ostream>
#include <vector>
#include <unordered_set>
#include <boost/thread.hpp>
#include <boost/ptr_container/ptr_list.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include "ftp/client.hpp"
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
  
  void AcceptClients();
  void AcceptClient(util::net::TCPListener& server);
  void HandleClients();
  void Run();
  
public:
  Listener() : port(-1) { }
  bool Initialise(const std::vector<std::string>& validIPs, int32_t port);
  
  void Stop();
};

}

#endif
