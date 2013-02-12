#include <algorithm>
#include <sys/types.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "util/net/interfaces.hpp"
#include "util/net/error.hpp"

namespace util { namespace net
{

void Interfaces::Load()
{
  struct ifaddrs* ifaddr, *ifa;
  if (getifaddrs(&ifaddr) == -1) return throw NetworkSystemError(errno);

  IPAddress ip;
  for (ifa = ifaddr; ifa; ifa = ifa->ifa_next)
  {
    if (!ifa->ifa_addr) continue;

    if (ifa->ifa_addr->sa_family == AF_INET)
    {
      struct sockaddr_in* addr4 =
        reinterpret_cast<struct sockaddr_in*>(ifa->ifa_addr);
      ip = IPAddress(&addr4->sin_addr, sizeof(addr4->sin_addr));
    }
    else if (ifa->ifa_addr->sa_family == AF_INET6)
    {
      struct sockaddr_in6* addr6 =
        reinterpret_cast<struct sockaddr_in6*>(ifa->ifa_addr);
      ip = IPAddress(&addr6->sin6_addr, sizeof(addr6->sin6_addr));
    }
    else
      continue;
    
    std::string name = ifa->ifa_name;
    std::map<std::string, Interface>::iterator it =
      interfaces.insert(std::make_pair(name, Interface(name))).first;
    it->second.addresses.emplace_back(ip);
  }
}

bool FindPartnerIP(const IPAddress& ip, IPAddress& partner)
{
  Interfaces interfaces;
  Interfaces::const_iterator it;
  for (it = interfaces.begin(); it != interfaces.end(); ++it)
  {
    Interface::const_iterator it2 = 
      std::find(it->second.begin(), it->second.end(), ip);
    if (it2 != it->second.end()) break;
  }
  
  if (it != interfaces.end())
  {
    for (Interface::const_iterator it2 = it->second.begin();
         it2 != it->second.end(); ++it2)
    {
      if (it2->Family() == IPFamily::IPv4)
      {
        partner = *it2;
        return true;
      }
    }
  }
  
  return false;
}

} /* net namespace */
} /* util namespace */
