#include <cstring>
#include <arpa/inet.h>
#include "util/net/ipaddress.hpp"
#include "util/error.hpp"

#include <iostream>

namespace util { namespace net
{

IPAddress::IPAddress(IPFamily family) :
  dataLen(0),
  family(family)
{
  if (family == IPFamily::IPv4) FromString("0.0.0.0");
  else if (family == IPFamily::IPv6) FromString("::");
  else throw InvalidIPAddressError();
}

IPAddress::IPAddress(const IPAddress& addr) :
  dataLen(addr.dataLen),
  family(addr.family)
{
  memcpy(data, addr.data, addr.dataLen);
}

IPAddress::IPAddress(const std::string& addr) :
  dataLen(0)
{
  FromString(addr);
}

IPAddress::IPAddress(const void *addr, socklen_t addrLen) :
  dataLen(addrLen)
{
  memcpy(data, addr, addrLen);
  if (addrLen == sizeof(in_addr)) family = IPFamily::IPv4;
  else if (addrLen == sizeof(in6_addr)) family = IPFamily::IPv6;
  else throw InvalidIPAddressError();
}

void IPAddress::FromString(const std::string& addr)
{
  memset(data, 0, sizeof(data));
  if (inet_pton(static_cast<int>(IPFamily::IPv4), 
                addr.c_str(), reinterpret_cast<struct in_addr*>(data)))
  {
    family = IPFamily::IPv4;
    dataLen = sizeof(struct in_addr);
  }
  else if (inet_pton(static_cast<int>(IPFamily::IPv6), 
                     addr.c_str(), reinterpret_cast<struct in6_addr*>(data)))
  {
    family = IPFamily::IPv6;
    dataLen = sizeof(struct in6_addr);
  }
  else
  {
    throw InvalidIPAddressError();
  }
  
  asString = addr;
}

void IPAddress::ToStringv4() const
{
  char str[INET_ADDRSTRLEN];
  if (!inet_ntop(static_cast<int>(IPFamily::IPv4),
                 data, str, sizeof(str))) 
    throw NetworkSystemError(errno);
  asString = str;
}

void IPAddress::ToStringv6() const
{
  char str[INET6_ADDRSTRLEN];
  if (!inet_ntop(static_cast<int>(IPFamily::IPv6),
                 data, str, sizeof(str)))
    throw NetworkSystemError(errno);
  asString = str;
}

const std::string& IPAddress::ToString() const
{
  if (!asString.empty()) return asString;
  if (family == IPFamily::IPv4) ToStringv4();
  else ToStringv6();
  return asString;
}

bool IPAddress::IsMappedv4() const
{
  if (family != IPFamily::IPv6) return false;
  return IN6_IS_ADDR_V4MAPPED(data);
}

IPAddress IPAddress::ToUnmappedv4() const
{
  struct in_addr addr;
  socklen_t len = sizeof(addr);
  memcpy(&addr, data + (sizeof(in6_addr) - len), len);
  return IPAddress(&addr, len);
}

bool IPAddress::Validv6(const std::string& addr)
{
  try
  {
    return IPAddress(addr).Family() == IPFamily::IPv6;
  }
  catch (const InvalidIPAddressError&)
  {
  }
  
  return false;
}

bool IPAddress::Validv4(const std::string& addr)
{
  try
  {
    return IPAddress(addr).Family() == IPFamily::IPv4;
  }
  catch (const InvalidIPAddressError&)
  {
  }
  
  return false;
}

bool IPAddress::Valid(const std::string& addr)
{
  try
  {
    IPAddress ip(addr);
  }
  catch (const InvalidIPAddressError&)
  {
    return false;
  }
  
  return true;

}

bool IPAddress::Equals(const IPAddress& addr) const
{
  if (IsMappedv4()) return ToUnmappedv4().Equals(addr);
  if (addr.IsMappedv4()) return Equals(addr.ToUnmappedv4());
  if (dataLen != addr.dataLen) return false;
  return !memcmp(data, addr.data, dataLen);
}

std::ostream& operator<<(std::ostream& os, const IPAddress& ip)
{
  return (os << ip.ToString());
}

} /* net namespace */
} /* util namespace */

#ifdef UTIL_NET_IPADDRESS_TEST

#include <iostream>

int main()
{
  using namespace util::net;
  
  IPAddress ip("127.0.0.1");
  
  std::cout << ip.ToString() << std::endl; 
  std::cout << ip << std::endl;
  std::cout << IPAddress::Valid("127.0.0.1") << std::endl;
  std::cout << IPAddress::Validv4("127.0.0.1") << std::endl;
  std::cout << IPAddress::Validv6("::") << std::endl;
  std::cout << (IPAddress("127.0.0.1") == IPAddress("127.0.0.1")) << std::endl;
  std::cout << (IPAddress("192.168.1.1") == IPAddress("127.0.0.1")) << std::endl;
}

#endif
