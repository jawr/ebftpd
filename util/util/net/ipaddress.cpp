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

#include <cstring>
#include <sys/types.h>
#include <arpa/inet.h>
#include "util/net/ipaddress.hpp"
#include "util/error.hpp"

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
  memcpy(&data, &addr.data, addr.dataLen);
}

IPAddress::IPAddress(const std::string& addr) :
  dataLen(0)
{
  FromString(addr);
}

IPAddress::IPAddress(const void *addr, socklen_t addrLen) :
  dataLen(addrLen)
{
  memcpy(&data, addr, addrLen);
  if (addrLen == sizeof(in_addr)) family = IPFamily::IPv4;
  else if (addrLen == sizeof(in6_addr)) family = IPFamily::IPv6;
  else throw InvalidIPAddressError();
}

void IPAddress::FromString(const std::string& addr)
{
  memset(&data, 0, sizeof(data));
  if (inet_pton(static_cast<int>(IPFamily::IPv4), addr.c_str(), &data.in4))
  {
    family = IPFamily::IPv4;
    dataLen = sizeof(struct in_addr);
  }
  else if (inet_pton(static_cast<int>(IPFamily::IPv6), addr.c_str(), &data.in6))
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
                 &data, str, sizeof(str))) 
    throw NetworkSystemError(errno);
  asString = str;
}

void IPAddress::ToStringv6() const
{
  char str[INET6_ADDRSTRLEN];
  if (!inet_ntop(static_cast<int>(IPFamily::IPv6),
                 &data, str, sizeof(str)))
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

bool IPAddress::IsLoopback() const
{
  if (family == IPFamily::IPv4)
  {
    return data.in4.s_addr == htonl(INADDR_LOOPBACK);
  }
  else
  if (IsMappedv4())
  {
    return ToUnmappedv4().IsLoopback();
  }
  else
  {
    return IN6_IS_ADDR_LOOPBACK(&data.in6);
  }
}

bool IPAddress::IsMappedv4() const
{
  if (family != IPFamily::IPv6) return false;
  return IN6_IS_ADDR_V4MAPPED(&data.in6);
}

IPAddress IPAddress::ToUnmappedv4() const
{
  return IPAddress(&data.unmapped.in4, sizeof(data.unmapped.in4));
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
  return !memcmp(&data, &addr.data, dataLen);
}

std::ostream& operator<<(std::ostream& os, const IPAddress& ip)
{
  return (os << ip.ToString());
}

} /* net namespace */
} /* util namespace */
