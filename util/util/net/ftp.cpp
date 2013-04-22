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

#include <vector>
#include <sstream>
#include <cstdint>
#include <boost/lexical_cast.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include "util/string.hpp"
#include <netinet/in.h>
#include "util/net/ftp.hpp"
#include "util/error.hpp"
#include "util/net/endpoint.hpp"

namespace util { namespace net { namespace ftp
{

using namespace util::net;

util::Error EndpointFromPORT(const std::string& portString, Endpoint& ep)
{
  using boost::numeric_cast;
  using boost::lexical_cast;

  std::vector<std::string> split;
  util::Split(split, portString, ",");
  if (split.size() != 6) return util::Error::Failure("Invalid port string.");

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  
  try
  {
    uint8_t* a = reinterpret_cast<uint8_t*>(&addr.sin_addr);
    a[0] = numeric_cast<uint8_t>(lexical_cast<int16_t>(split[0]));
    a[1] = numeric_cast<uint8_t>(lexical_cast<int16_t>(split[1]));
    a[2] = numeric_cast<uint8_t>(lexical_cast<int16_t>(split[2]));
    a[3] = numeric_cast<uint8_t>(lexical_cast<int16_t>(split[3]));
    
    uint8_t* p = reinterpret_cast<uint8_t*>(&addr.sin_port);
    p[0] = numeric_cast<uint8_t>(lexical_cast<int16_t>(split[4]));
    p[1] = numeric_cast<uint8_t>(lexical_cast<int16_t>(split[5]));
  }
  // neither of these exceptions should ever happen
  catch (const std::bad_cast&)
  {
    return util::Error::Failure("Unknown error");
  }  
  
  try
  {
    ep = Endpoint(addr);  
  }
  catch (const NetworkSystemError& e)
  {
    return util::Error::Failure(e.Errno());
  }
  catch (const NetworkError& e)
  {
    return util::Error::Failure(e.Message());
  }
  
  return util::Error::Success();
}

util::Error EndpointToPORTv4Mappedv6(const Endpoint& ep, std::string& portString)
{
  const struct sockaddr_in6* addr6 = 
    reinterpret_cast<const struct sockaddr_in6*>(ep.Addr());

  const uint8_t* a = reinterpret_cast<const uint8_t*>(&addr6->sin6_addr);
  const uint8_t* p = reinterpret_cast<const uint8_t*>(&addr6->sin6_port);
  
  std::ostringstream os;
  // must cast to int as uint8_t doesn't output as numberic value (char)
  os << int(a[12]) << ',' << int(a[13]) << ','
     << int(a[14]) << ',' << int(a[15]) << ','
     << int(p[ 0]) << ',' << int(p[ 1]);
  
  portString = os.str();
  return util::Error::Success();
}

util::Error EndpointToPORTv4(const Endpoint& ep, std::string& portString)
{
  const struct sockaddr_in* addr4 = 
    reinterpret_cast<const struct sockaddr_in*>(ep.Addr());

  const uint8_t* a = reinterpret_cast<const uint8_t*>(&addr4->sin_addr);
  const uint8_t* p = reinterpret_cast<const uint8_t*>(&addr4->sin_port);
  
  std::ostringstream os;
  // must cast to int as uint8_t doesn't output as numberic value (char)
  os << int(a[0]) << ',' << int(a[1]) << ','
     << int(a[2]) << ',' << int(a[3]) << ','
     << int(p[0]) << ',' << int(p[1]);
  
  portString = os.str();
  return util::Error::Success();
}

util::Error EndpointToPORT(const Endpoint& ep, std::string& portString)
{
  if (ep.IP().Family() != IPFamily::IPv4)
  {
    const struct sockaddr_in6* addr6 = 
      reinterpret_cast<const struct sockaddr_in6*>(ep.Addr());
    if (IN6_IS_ADDR_V4MAPPED(&addr6->sin6_addr))
      return EndpointToPORTv4Mappedv6(ep, portString);
    else
      return util::Error::Failure(EPFNOSUPPORT);
  }
  else
    return EndpointToPORTv4(ep, portString);
} 

util::Error EndpointFromLPRTv4(const std::string& portString, Endpoint& ep)
{
  using boost::lexical_cast;
  using boost::numeric_cast;

  std::vector<std::string> split;
  util::Split(split, portString, ",");
  if (split.size() != 9) return util::Error::Failure("Invalid port string.");
  
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  
  try
  {
    uint8_t addrLen = numeric_cast<uint8_t>(lexical_cast<int16_t>(split[1]));
    if (addrLen != 4) return util::Error::Failure("Invalid port string.");
    
    uint8_t* a = reinterpret_cast<uint8_t*>(&addr.sin_addr);
    a[0] = numeric_cast<uint8_t>(lexical_cast<int16_t>(split[2]));
    a[1] = numeric_cast<uint8_t>(lexical_cast<int16_t>(split[3]));
    a[2] = numeric_cast<uint8_t>(lexical_cast<int16_t>(split[4]));
    a[3] = numeric_cast<uint8_t>(lexical_cast<int16_t>(split[5]));
    
    uint8_t portLen = numeric_cast<uint8_t>(lexical_cast<int16_t>(split[6]));
    if (portLen != 2) return util::Error::Failure("Invalid port string.");

    uint8_t* p = reinterpret_cast<uint8_t*>(&addr.sin_port);
    p[0] = numeric_cast<uint8_t>(lexical_cast<int16_t>(split[7]));
    p[1] = numeric_cast<uint8_t>(lexical_cast<int16_t>(split[8]));
  }
  catch (const std::bad_cast&)
  {
    return util::Error::Failure("Invalid port string.");
  }  
  
  try
  {
    ep = Endpoint(addr);  
  }
  catch (const NetworkSystemError& e)
  {
    return util::Error::Failure(e.Errno());
  }
  catch (const NetworkError& e)
  {
    return util::Error::Failure(e.Message());
  }
  
  return util::Error::Success();  
}

util::Error EndpointFromLPRTv6(const std::string& portString, Endpoint& ep)
{
  using boost::lexical_cast;
  using boost::numeric_cast;

  std::vector<std::string> split;
  util::Split(split, portString, ",");
  if (split.size() != 21) return util::Error::Failure("Invalid port string.");
  
  struct sockaddr_in6 addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin6_family = AF_INET6;
  
  try
  {
    uint8_t addrLen = numeric_cast<uint8_t>(lexical_cast<int16_t>(split[1]));
    if (addrLen != 16) return util::Error::Failure("Invalid port string.");
    
    uint8_t* a = reinterpret_cast<uint8_t*>(&addr.sin6_addr);
    for (unsigned i = 0, j = 2; i < 16; ++i, ++j)
      a[i] = numeric_cast<uint8_t>(lexical_cast<int16_t>(split[j]));
    
    uint8_t portLen = numeric_cast<uint8_t>(lexical_cast<int16_t>(split[18]));
    if (portLen != 2) return util::Error::Failure("Invalid port string.");

    uint8_t* p = reinterpret_cast<uint8_t*>(&addr.sin6_port);
    p[0] = numeric_cast<uint8_t>(lexical_cast<int16_t>(split[19]));
    p[1] = numeric_cast<uint8_t>(lexical_cast<int16_t>(split[20]));
  }
  catch (const std::bad_cast&)
  {
    return util::Error::Failure("Invalid port string.");
  }  
  
  try
  {
    ep = Endpoint(addr);  
  }
  catch (const NetworkSystemError& e)
  {
    return util::Error::Failure(e.Errno());
  }
  catch (const NetworkError& e)
  {
    return util::Error::Failure(e.Message());
  }
  
  return util::Error::Success();  
}

util::Error EndpointFromLPRT(const std::string& portString, Endpoint& ep)
{
  if (portString.empty()) return util::Error::Failure("Empty port string");
  if (portString[0] == '4')
    return EndpointFromLPRTv4(portString, ep);
  else if (portString[0] == '6')
    return EndpointFromLPRTv6(portString, ep);
  return util::Error::Failure(EPFNOSUPPORT);
}

void EndpointToLPRTv4(const Endpoint& ep, std::string& portString)
{
  const struct sockaddr_in* addr4 = 
    reinterpret_cast<const struct sockaddr_in*>(ep.Addr());

  const uint8_t* a = reinterpret_cast<const uint8_t*>(&addr4->sin_addr);
  const uint8_t* p = reinterpret_cast<const uint8_t*>(&addr4->sin_port);
  
  std::ostringstream os;
  // must cast to int as uint8_t doesn't output as numberic value (char)
  os << 4         << ',' << 4         << ',' << int(a[0]) << ','
     << int(a[1]) << ',' << int(a[2]) << ',' << int(a[3]) << ','
     << 2         << ',' << int(p[0]) << ',' << int(p[1]);
  
  portString = os.str();
}

void EndpointToLPRTv6(const Endpoint& ep, std::string& portString)
{
  const struct sockaddr_in6* addr6 = 
    reinterpret_cast<const struct sockaddr_in6*>(ep.Addr());

  const uint8_t* a = reinterpret_cast<const uint8_t*>(&addr6->sin6_addr);
  const uint8_t* p = reinterpret_cast<const uint8_t*>(&addr6->sin6_port);
  
  std::ostringstream os;
  // must cast to int as uint8_t doesn't output as numberic value (char)
  os << 6          << ',' << 16         << ',' << int(a[ 0]) << ','
     << int(a[ 1]) << ',' << int(a[ 2]) << ',' << int(a[ 3]) << ','
     << int(a[ 4]) << ',' << int(a[ 5]) << ',' << int(a[ 6]) << ','
     << int(a[ 7]) << ',' << int(a[ 8]) << ',' << int(a[ 9]) << ','
     << int(a[10]) << ',' << int(a[11]) << ',' << int(a[12]) << ','
     << int(a[13]) << ',' << int(a[14]) << ',' << int(a[15]) << ','
     << 2          << ',' << int(p[ 0]) << ',' << int(p [1]);
  
  portString = os.str();
}

void EndpointToLPRT(const Endpoint& ep, std::string& portString)
{
  if (ep.IP().Family() == IPFamily::IPv4)
    EndpointToLPRTv4(ep, portString);
  else
    EndpointToLPRTv6(ep, portString);
}

util::Error EndpointFromEPRT(const std::string& portString, util::net::Endpoint& ep)
{
  using boost::numeric_cast;
  using boost::lexical_cast;

  std::vector<std::string> split;
  util::Split(split, portString, "|");
  if (split.size() != 5 || !split[0].empty() || !split[4].empty())
    return util::Error::Failure("Invalid port string.");
  
  uint8_t family;
  uint16_t port;
  try
  {
    family = numeric_cast<uint8_t>(lexical_cast<int16_t>(split[1]));
    port = lexical_cast<uint16_t>(split[3]);
  }
  catch (const std::bad_cast&)
  {
    return util::Error::Failure("Invalid port string.");
  }
  
  if (family != 1 && family != 2)
    return util::Error::Failure("Invalid port string.");
    
  try
  {
    ep = Endpoint(split[2], port);  
  }
  catch (const NetworkSystemError& e)
  {
    return util::Error::Failure(e.Errno());
  }
  catch (const NetworkError& e)
  {
    return util::Error::Failure(e.Message());
  }
  
  return util::Error::Success();    
}

void EndpointToEPRT(const Endpoint& ep, std::string& portString, bool full)
{
  int16_t family = ep.IP().Family() == util::net::IPFamily::IPv4 ? 1 : 2;
  
  std::ostringstream os;
  if (full)
    os << "|" << family
       << "|" << ep.IP()
       << "|" << ep.Port() << "|";
  else
    os << "|||" << ep.Port() << "|";
    
  portString = os.str();
}

} /* ftp namespace */
} /* net namespace */
} /* util namespace */
