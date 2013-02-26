#ifndef __UTIL_NET_FTP_HPP
#define __UTIL_NET_FTP_HPP

#include <string>

namespace util
{
class Error;

namespace net
{ 
class Endpoint;

namespace ftp
{

util::Error EndpointFromPORT(const std::string& portString, util::net::Endpoint& ep);
util::Error EndpointToPORT(const util::net::Endpoint& ep, std::string& portString);
util::Error EndpointFromLPRT(const std::string& portString, util::net::Endpoint& ep);
void EndpointToLPRT(const util::net::Endpoint& ep, std::string& portString);
util::Error EndpointFromEPRT(const std::string& portString, util::net::Endpoint& ep);
void EndpointToEPRT(const util::net::Endpoint& ep, std::string& portString, bool full = false);

} /* ftp namespace */
} /* net namespace */
} /* util namespace */

#endif
