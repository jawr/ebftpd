#ifndef __UTIL_NET_TCPLISTENER_HPP
#define __UTIL_NET_TCPLISTENER_HPP

#include <mutex>
#include <sys/socket.h>
#include <boost/noncopyable.hpp>
#include "util/net/endpoint.hpp"
#include "util/timepair.hpp"

namespace util {namespace net
{

class TCPSocket;

class TCPListener : boost::noncopyable
{
  util::net::Endpoint endpoint;
  std::mutex socketMutex;
  int socket;
  int backlog;

  TCPListener(const TCPListener&) = delete;
  TCPListener& operator=(const TCPListener&) = delete;
  
  void Listen();
  
public:
  static const int maximumBacklog = SOMAXCONN;

  ~TCPListener();
  
	TCPListener(int backlog = maximumBacklog);
  /* Throws NetworkSystemError */
  
	TCPListener(const Endpoint& endpoint, int backlog = maximumBacklog);
  /* Throws NetworkSystemError, InvalidIPAddressError */
                
  void Listen(const Endpoint& endpoint);
  /* Throws NetworkSystemError, InvalidIPAddressError */
  
  void Accept(TCPSocket& socket);
  /* Throws NetworkSystemError, InvalidIPAddressError */
  
  void Close();
  /* No exceptions */
  
  int Socket() const { return socket; }
  /* No exceptions */
  
  void Shutdown();

  bool IsListening() const { return socket >= 0; }
  
  const util::net::Endpoint& Endpoint() const { return endpoint; }
  /* No exceptions */
};
  
} /* net namespace */
} /* util namespace */

#endif
