#ifndef __UTIL_NET_TCPLISTENER_HPP
#define __UTIL_NET_TCPLISTENER_HPP

#include <sys/socket.h>
#include "util/net/endpoint.hpp"
#include "util/timepair.hpp"

namespace util {namespace net
{

class TCPSocket;

class TCPListener
{
  util::net::Endpoint endpoint;
  int socket;
  int backlog;
  int interruptPipe[2];
  
  bool WaitPendingTimeout(const TimePair* duration) const;
  
public:
  static const int maximumBacklog = SOMAXCONN;

	TCPListener(const Endpoint& endpoint, int backlog = maximumBacklog);
              
	TCPListener(int backlog = maximumBacklog);
  
  void Listen();
  void Listen(const Endpoint& endpoint);
  
  void Accept(TCPSocket& socket);
  
  bool WaitPendingTimeout(const TimePair& duration) const;
  bool WaitPending() const;
  bool Pending() const;
  
  int Socket() const { return socket; }
  const util::net::Endpoint& Endpoint() const
  { return endpoint; }
  
};
  
} /* net namespace */
} /* util namespace */

#endif
