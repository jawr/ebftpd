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

#ifndef __UTIL_NET_IDENTCLIENT_HPP
#define __UTIL_NET_IDENTCLIENT_HPP

#include <cstdint>
#include <string>
#include <boost/noncopyable.hpp>
#include "util/net/tcpsocket.hpp"
#include "util/net/endpoint.hpp"

namespace util { namespace net
{

class TCPSocket;
 
class IdentClient : boost::noncopyable
{
  TCPSocket socket;
  Endpoint localEndpoint;
  Endpoint remoteEndpoint;
  
  std::string os;
  std::string ident;
  
  static const uint16_t identPort = 113;
  static const util::TimePair defaultTimeout; // 15 seconds
  
  void Request();
  
public:
  IdentClient(const util::TimePair& timeout = defaultTimeout) :
    socket(timeout) { }
  IdentClient(const Endpoint& localEndpoint, const Endpoint& remoteEndpoint,
              const util::TimePair& timeout = defaultTimeout);
  IdentClient(const TCPSocket& client,
              const util::TimePair& timeout = defaultTimeout);
    
  void Request(const Endpoint& localEndpoint, const Endpoint& remoteEndpoint);
    
  const std::string& OS() const { return os; }
  const std::string& Ident() const { return ident; }
};

} /* net namespace */
} /* util namespace */

#endif
