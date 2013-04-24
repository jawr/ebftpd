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
