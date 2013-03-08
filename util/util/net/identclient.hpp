#ifndef __UTIL_NET_IDENTCLIENT_HPP
#define __UTIL_NET_IDENTCLIENT_HPP

#include <cstdint>
#include <string>
#include <boost/noncopyable.hpp>
#include "util/net/tcpsocket.hpp"
#include "util/net/endpoint.hpp"

namespace util { namespace net
{

class TCPSocket;
 
class IdentClient : boost::noncopyable
{
  TCPSocket socket;
  Endpoint localEndpoint;
  Endpoint remoteEndpoint;
  
  std::string os;
  std::string ident;
  
  static const uint16_t identPort = 113;
  static const util::TimePair defaultTimeout; // 15 seconds
  
  void Request();
  
public:
  IdentClient(const util::TimePair& timeout = defaultTimeout) :
    socket(timeout) { }
  IdentClient(const Endpoint& localEndpoint, const Endpoint& remoteEndpoint,
              const util::TimePair& timeout = defaultTimeout);
  IdentClient(const TCPSocket& client,
              const util::TimePair& timeout = defaultTimeout);
    
  void Request(const Endpoint& localEndpoint, const Endpoint& remoteEndpoint);
    
  const std::string& OS() const { return os; }
  const std::string& Ident() const { return ident; }
};

} /* net namespace */
} /* util namespace */

#endif
