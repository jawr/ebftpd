//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <algorithm>
#include <memory>
#include <sys/types.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include "util/net/interfaces.hpp"
#include "util/net/error.hpp"

namespace util { namespace net
{

void Interfaces::Load()
{
  struct ifaddrs* ifaddr, *ifa;
  if (getifaddrs(&ifaddr) == -1) return throw NetworkSystemError(errno);
  std::shared_ptr<struct ifaddrs> ifaddrGuard(ifaddr, freeifaddrs);

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
