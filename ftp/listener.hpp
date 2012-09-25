#ifndef __FTP_LISTENER_HPP
#define __FTP_LISTENER_HPP
#include <ostream>
#include <tr1/unordered_set>
#include <boost/thread.hpp>
#include <boost/ptr_container/ptr_list.hpp>
#include "util/endpoint.hpp"
#include "ftp/client.hpp"
#include "util/thread.hpp"
#include "util/tcpserver.hpp"
#include "util/tcpclient.hpp"
namespace ftp
{
class Listener : public util::ThreadSelect
{
  typedef boost::ptr_list<Client> ClientList;

  util::endpoint addr;
  util::tcp::server server;
  boost::ptr_list<Client> clients;
  
  void AcceptClients();
  void AcceptClient();
  void HandleClients();
  
public:
  Listener() : addr("0.0.0.0", 21), server(), clients() {};
  Listener(const std::string& ip, uint16_t port) : addr(ip, port), server() {};
  ~Listener() {};
  void Run();
  bool Initialise();
};
}
#endif
