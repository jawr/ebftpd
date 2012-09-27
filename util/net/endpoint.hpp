#ifndef __UTIL_NET_ENDPOINT_HPP
#define __UTIL_NET_ENDPOINT_HPP

#include <algorithm>
#include <cstring>
#include <ostream>
#include <sys/socket.h>
#include <boost/cstdint.hpp>
#include "util/net/ipaddress.hpp"

namespace util { namespace net
{

class Endpoint
{
  struct sockaddr_storage addr;
  struct sockaddr_in* addr4;
  struct sockaddr_in6* addr6;
  IPAddress ip;
  socklen_t addrLen;
  int32_t port;
  
  bool Equals(const Endpoint& ep) const;
  void FromAddr(const struct sockaddr& addr);

public:
  Endpoint();
  /*  No exceptions */

  Endpoint(const std::string& addr, int32_t port);
  /* Throws InvalidIPAddressError */
  
  Endpoint(const IPAddress& addr, int32_t port);
  /* No exceptions */

  Endpoint(const struct sockaddr_in& addr);
  /* Throws InvalidIPAddressError */

  Endpoint(const struct sockaddr_in6& addr);
  /* Throws InvalidIPAddressError */

  Endpoint(const struct sockaddr& addr, socklen_t addrLen);
  /* Throws InvalidIPAddressError */
  
  socklen_t Length() const { return addrLen; }
  /*  No exceptions */

  const struct sockaddr* Addr() const;
  /*  No exceptions */

  const IPAddress& IP() const { return ip; }
  /*  No exceptions */

  int32_t Port() const { return port; }
  /*  No exceptions */

  bool Empty() const { return !addr4 && !addr6; }
  /*  No exceptions */

  IPAddress::IPFamily Family() const { return ip.Family(); }
  /*  No exceptions */
  
  bool operator==(const Endpoint& lhs) const { return Equals(lhs); }
  /*  No exceptions */

  bool operator!=(const Endpoint& lhs) const { return !Equals(lhs); }
  /*  No exceptions */
  
  static int32_t AnyPort() { return 0; }
  /*  No exceptions */
  
  std::string ToString() const;
};

std::ostream& operator<<(std::ostream& os, const Endpoint& ep);

} /* net namespace */
} /* util namespace */

#endif
