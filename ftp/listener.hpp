#ifndef __FTP_LISTENER_HPP
#define __FTP_LISTENER_HPP

#include <ostream>
#include <vector>
#include <tr1/unordered_set>
#include <boost/thread.hpp>
#include <boost/ptr_container/ptr_list.hpp>
#include "ftp/client.hpp"
#include "util/thread.hpp"
#include "util/net/tcplistener.hpp"
#include "util/net/tcpsocket.hpp"

namespace ftp
{

class Listener : public util::ThreadSelect
{
  typedef boost::ptr_list<Client> ClientList;

  std::vector<util::net::TCPListener> servers;
  boost::ptr_list<Client> clients;
  std::vector<std::string> validIPs;
  int32_t port;
  
  void AcceptClients();
  void AcceptClient(util::net::TCPListener& server);
  void HandleClients();
  void Run();
  
public:
  Listener() : port(-1) { }
  bool Initialise(const std::vector<std::string>& validIPs, int32_t port);
};

}

#endif
