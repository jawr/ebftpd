#include "util/net/endpoint.hpp"

namespace util { namespace net
{

Endpoint::Endpoint() :
  addr4(0),
  addr6(0),
  addrLen(0),
  port(-1)
{
}

Endpoint::Endpoint(const std::string& addr, int32_t port) :
  ip(addr),
  addrLen(0),
  port(port)
{
  memset(&this->addr, 0, sizeof(this->addr));
  if (ip.Family() == IPAddress::IPv4)
  {
    addr4 = static_cast<struct sockaddr_in*>(static_cast<void*>(&this->addr));
    memcpy(&addr4->sin_addr, ip.Addr(), ip.Length());
    addrLen = sizeof(struct sockaddr_in);
    addr4->sin_family = AF_INET;
    addr4->sin_port = htons(port);
  }
  else if (ip.Family() == IPAddress::IPv6)
  {
    addr6 = static_cast<struct sockaddr_in6*>(static_cast<void*>(&this->addr));
    memcpy(&addr6->sin6_addr, ip.Addr(), ip.Length());
    addrLen = sizeof(struct sockaddr_in6);
    addr6->sin6_family = AF_INET6;
    addr6->sin6_family = htons(port);
  }
}

Endpoint::Endpoint(const struct sockaddr& addr, socklen_t addrLen) :
  addr4(0),
  addr6(0),
  addrLen(addrLen),
  port(-1)
{
  memcpy(&this->addr, &addr, addrLen);
  if (addrLen == sizeof(struct sockaddr_in))
  {
    addr4 = (struct sockaddr_in*) &this->addr;
    ip = IPAddress(&addr4->sin_addr, sizeof(addr4->sin_addr));
    port = ntohs(addr4->sin_port);
  }
  else if (addrLen == sizeof(struct sockaddr_in6))
  {
    addr6 = (struct sockaddr_in6*) &this->addr;
    ip = IPAddress(&addr6->sin6_addr, sizeof(addr6->sin6_addr));
    port = ntohs(addr6->sin6_port);
  }
}

const struct sockaddr* Endpoint::Addr() const
{
  return static_cast<const struct sockaddr*>(static_cast<const void*>(&addr));
}

bool Endpoint::Equals(const Endpoint& ep) const
{
  return ip == ep.ip && port == ep.port;
}

std::ostream& operator<<(std::ostream& os, const Endpoint& ep)
{
  if (ep.Family() == IPAddress::IPv6)
    return (os << "[" << ep.IP() << "]" << ":" << ep.Port());
  else
    return (os << ep.IP() << ":" << ep.Port());
}

} /* net namespace */
} /* util namespace */

#ifdef UTIL_NET_ENDPOINT_TEST

#include <iostream>

int main()
{
  using namespace util::net;
  
  Endpoint ep("127.0.0.1", 1337);
  
  std::cout << ep.IP() << " " << ep.Port() << std::endl;
  std::cout << ep << std::endl;
  std::cout << (ep == Endpoint("127.0.0.1", 1337)) << std::endl;
  std::cout << (ep == Endpoint("127.0.0.1", 1234)) << std::endl;
  std::cout << Endpoint("::", 1313) << std::endl;
}

#endif
