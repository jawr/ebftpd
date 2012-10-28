#ifndef __UTIL_NET_RESOLVER_HPP
#define __UTIL_NET_RESOLVER_HPP

#include <vector>
#include <string>
#include <stdexcept>
#include <cstdint>
#include <boost/noncopyable.hpp>
#include <sys/socket.h>
#include "util/net/endpoint.hpp"
#include "util/net/error.hpp"

struct addrinfo;

namespace util { namespace net
{

enum class SocketType : int
{
  Stream = SOCK_STREAM,
  Datagram = SOCK_DGRAM
};

class ResolverError : public NetworkError
{
  int code;

public:
  ResolverError(int code);
  
  int Code() const { return code; }
};

class Resolver : boost::noncopyable
{
  SocketType socketType;
  std::string hostname;
  int32_t port;
  std::vector<Endpoint> results;
  struct addrinfo* res;
  
  void Resolve();

  public:
  typedef std::vector<Endpoint>::const_iterator const_iterator;
  
  ~Resolver();
    
  Resolver(SocketType socketType);
  /* No exceptions */

  Resolver(SocketType socketType, const std::string& hostname, int32_t port);  
  /* Throws ResolverError, NetworkSystemError, InvalidIPAddressError */

  void Resolve(const std::string& hostname, int32_t port);
  /* Throws ResolverError, NetworkSystemError, InvalidIPAddressError */
  
  const_iterator begin() const { return results.begin(); }
  /* No exceptions */

  const_iterator end() const { return results.end(); }
  /* No exceptions */
  
  static int32_t AnyPort() { return 0; }
  /* No exceptions */
};

class TCPResolver : public Resolver
{
public:
  TCPResolver() : Resolver(SocketType::Stream) { }
  /* No exceptions */
  
  TCPResolver(const std::string& hostname, int32_t port) : 
    Resolver(SocketType::Stream, hostname, port) { }
  /* Throws ResolverError, NetworkSystemError, InvalidIPAddressError */

};

class UDPResolver : public Resolver
{
public:
  UDPResolver() : Resolver(SocketType::Datagram) { }
  /* No exceptions */
  
  UDPResolver(const std::string& hostname, int32_t port) : 
    Resolver(SocketType::Datagram, hostname, port) { }
  /* Throws ResolverError, NetworkSystemError, InvalidIPAddressError */
};

} /* net namespace */
} /* util namespace */

#endif
