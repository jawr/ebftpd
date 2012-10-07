#include <sstream>
#include "util/net/endpoint.hpp"

namespace util { namespace net
{

Endpoint::Endpoint() :
  addr4(nullptr),
  addr6(nullptr),
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
  if (ip.Family() == IPFamily::IPv4)
  {
    addr4 = reinterpret_cast<struct sockaddr_in*>(&this->addr);
    memcpy(&addr4->sin_addr, ip.Addr(), ip.Length());
    addrLen = sizeof(struct sockaddr_in);
    addr4->sin_family = AF_INET;
    addr4->sin_port = htons(port);
  }
  else if (ip.Family() == IPFamily::IPv6)
  {
    addr6 = reinterpret_cast<struct sockaddr_in6*>(&this->addr);
    memcpy(&addr6->sin6_addr, ip.Addr(), ip.Length());
    addrLen = sizeof(struct sockaddr_in6);
    addr6->sin6_family = AF_INET6;
    addr6->sin6_port = htons(port);
  }
}

Endpoint::Endpoint(const IPAddress& addr, int32_t port) :
  ip(addr),
  addrLen(0),
  port(port)
{
  memset(&this->addr, 0, sizeof(this->addr));
  if (ip.Family() == IPFamily::IPv4)
  {
    addr4 = reinterpret_cast<struct sockaddr_in*>(&this->addr);
    memcpy(&addr4->sin_addr, ip.Addr(), ip.Length());
    addrLen = sizeof(struct sockaddr_in);
    addr4->sin_family = AF_INET;
    addr4->sin_port = htons(port);
  }
  else if (ip.Family() == IPFamily::IPv6)
  {
    addr6 = reinterpret_cast<struct sockaddr_in6*>(&this->addr);
    memcpy(&addr6->sin6_addr, ip.Addr(), ip.Length());
    addrLen = sizeof(struct sockaddr_in6);
    addr6->sin6_family = AF_INET6;
    addr6->sin6_port = htons(port);
  }
}

Endpoint::Endpoint(const struct sockaddr& addr, socklen_t addrLen) :
  addr4(nullptr),
  addr6(nullptr),
  addrLen(addrLen),
  port(-1)
{
  FromAddr(addr);
}

Endpoint::Endpoint(const struct sockaddr_in& addr) :
  addr4(nullptr),
  addr6(nullptr),
  addrLen(sizeof(addr)),
  port(-1)
{
  FromAddr(*reinterpret_cast<const struct sockaddr*>(&addr));
}

Endpoint::Endpoint(const struct sockaddr_in6& addr) :
  addr4(nullptr),
  addr6(nullptr),
  addrLen(sizeof(addr)),
  port(-1)
{
  FromAddr(*reinterpret_cast<const struct sockaddr*>(&addr));
}

void Endpoint::FromAddr(const struct sockaddr& addr)
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
  return reinterpret_cast<const struct sockaddr*>(&addr);
}

bool Endpoint::Equals(const Endpoint& ep) const
{
  return ip == ep.ip && port == ep.port;
}

std::string Endpoint::ToString() const
{
  std::ostringstream os;
  if (ip.Family() == IPFamily::IPv6)
    os << "[" << ip << "]" << ":" << port;
  else
    os << ip << ":" << port;
  return os.str();
}

std::ostream& operator<<(std::ostream& os, const Endpoint& ep)
{
  return (os << ep.ToString());
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
