#ifndef __FTP_LISTENER_HPP
#define __FTP_LISTENER_HPP
#include <ostream>
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

  util::net::Endpoint addr;
  util::net::TCPListener server;
  boost::ptr_list<Client> clients;
  
  void AcceptClients();
  void AcceptClient();
  void HandleClients();
  
public:
  Listener() : addr("0.0.0.0", 21) { }
  Listener(const std::string& ip, int32_t port) : addr(ip, port){ }
  ~Listener() { }
  void Run();
  bool Initialise();
};

}

#endif
