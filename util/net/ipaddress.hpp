#ifndef __UTIL_NET_IPADDRESS_HPP
#define __UTIL_NET_IPADDRESS_HPP

#include <string>
#include <ostream>
#include <stdexcept>
#include <netinet/in.h>
#include <sys/socket.h>
#include "util/net/error.hpp"

namespace util { namespace net
{

class InvalidIPAddressError : public NetworkError
{
public:
  InvalidIPAddressError() :
    std::runtime_error("Invalid IP address or family") { }
};

enum class IPFamily : int
{
  IPv4 = AF_INET,
  IPv6 = AF_INET6
};

class IPAddress
{
private:
  char data[sizeof(in6_addr)];
  socklen_t dataLen;
  IPFamily family;
  mutable std::string asString;
  
  void FromString(const std::string& addr);
  void ToStringv4() const;
  void ToStringv6() const;
  bool Equals(const IPAddress& addr) const;
  
public:
  IPAddress(IPFamily family = IPFamily::IPv4);
  /* Throws InvalidIPAddressError */
  
  IPAddress(const IPAddress& addr);
  /* No exceptions */
  
  IPAddress(const std::string& addr);
  /* Throws InvalidIPAddressError */
  
  IPAddress(const void *addr, socklen_t addrLen);
  /* Throws InvalidIPAddressError */
  
  IPFamily Family() const { return family; }
  /* No exceptions */
  
  const std::string& ToString() const;
  /* Throws util::NetworkSystemError */

  bool IsMappedv4() const;
  
  IPAddress ToUnmappedv4() const;
  
  const void* Addr() const { return static_cast<const void*>(data); }
  /* No exceptions */

  socklen_t Length() const { return dataLen; }
  /* No exceptions */
  
  static IPAddress Any(IPFamily family) { return IPAddress(family); }
  /* No exceptions */

  static bool Validv6(const std::string& addr);
  /* No exceptions */
  
  static bool Validv4(const std::string& addr);
  /* No exceptions */

  static bool Valid(const std::string& addr);
  /* No exceptions */
  
  bool operator==(const IPAddress& lhs) const { return Equals(lhs); }
  /* No exceptions */

  bool operator!=(const IPAddress& lhs) const { return !Equals(lhs); }
  /* No exceptions */
};

std::ostream& operator<<(std::ostream& os, const IPAddress& ip);

} /* net namespace */
} /* util namespace */

#endif
