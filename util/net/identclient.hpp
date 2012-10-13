#ifndef __UTIL_NET_IDENTCLIENT_HPP
#define __UTIL_NET_IDENTCLIENT_HPP

#include <cstdint>
#include <string>
#include "util/net/tcpsocket.hpp"
#include "util/net/endpoint.hpp"

namespace util { namespace net
{

class TCPSocket;

class IdentClient
{
  TCPSocket socket;
  Endpoint localEndpoint;
  Endpoint remoteEndpoint;
  
  std::string os;
  std::string ident;
  
  static const uint16_t identPort = 113;
  
  void Request();
  
public:
  IdentClient() { }
  IdentClient(const Endpoint& localEndpoint, const Endpoint& remoteEndpoint);
  IdentClient(const TCPSocket& client);
    
  void Request(const Endpoint& localEndpoint, const Endpoint& remoteEndpoint);
    
  const std::string& OS() const { return os; }
  const std::string& Ident() const { return ident; }
};

} /* net namespace */
} /* util namespace */

#endif
