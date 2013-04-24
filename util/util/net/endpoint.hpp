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

#ifndef __UTIL_NET_ENDPOINT_HPP
#define __UTIL_NET_ENDPOINT_HPP

#include <cstring>
#include <ostream>
#include <sys/socket.h>
#include <cstdint>
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

  IPFamily Family() const { return ip.Family(); }
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
