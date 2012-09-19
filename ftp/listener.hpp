#ifndef __FTP_LISTENER_HPP
#define __FTP_LISTENER_HPP
#include <ostream>
#include <tr1/unordered_set>
#include <boost/thread.hpp>
#include "util/endpoint.hpp"
#include "util/thread.hpp"
#include "util/tcpserver.hpp"
#include "util/tcpclient.hpp"
namespace ftp
{
class Listener : public util::Thread
{
  util::endpoint addr;
  util::tcp::server server;
  std::tr1::unordered_set<util::tcp::client> clients;
  
public:
  Listener() : Thread(), addr("127.0.0.1", 21), server(), clients() {};
  Listener(const std::string& ip, uint16_t port) : 
    Thread(), addr(ip, port), server() {};
  ~Listener() {};
  void Run();
  void Listen();

};
}
#endif
