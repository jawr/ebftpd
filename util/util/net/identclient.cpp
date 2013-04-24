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

#include <sstream>
#include "util/string.hpp"
#include "util/net/identclient.hpp"

namespace util { namespace net
{

const util::TimePair IdentClient::defaultTimeout(15, 0);

IdentClient::IdentClient(const Endpoint& localEndpoint,
                         const Endpoint& remoteEndpoint,
                         const util::TimePair& timeout) :
  socket(Endpoint(remoteEndpoint.IP(), identPort), timeout),
  localEndpoint(localEndpoint),
  remoteEndpoint(remoteEndpoint)
{
  Request();
}

IdentClient::IdentClient(const TCPSocket& client, const util::TimePair& timeout) :
  socket(Endpoint(client.RemoteEndpoint().IP(), identPort), timeout),
  localEndpoint(client.LocalEndpoint()),
  remoteEndpoint(client.RemoteEndpoint())
{
  Request();
}

void IdentClient::Request(const Endpoint& localEndpoint,
                          const Endpoint& remoteEndpoint)
{
  this->localEndpoint = localEndpoint;
  this->remoteEndpoint = remoteEndpoint;
  socket.Connect(Endpoint(remoteEndpoint.IP(), identPort));
  Request();
}

void IdentClient::Request()
{
  std::ostringstream buf;
  buf << remoteEndpoint.Port() << ", " << localEndpoint.Port() << "\r\n";
  socket.Write(buf.str().c_str(), buf.str().length());
  
  std::string response;
  socket.Getline(response, true);

  std::vector<std::string> toks;
  util::Split(toks, response, ":");
  if (toks.size() < 3) throw NetworkError("Malformed ident response");
  
  util::Trim(toks[0]);
  std::stringstream is(toks[0]);

  uint16_t remotePort;
  is >> remotePort;
  char comma;
  is >> comma;
  uint16_t localPort;
  is >> localPort;
  
  if (!is.eof() || comma != ',')
    throw NetworkError("Malformed ident response");
    
  util::ToUpper(toks[1]);
  util::Trim(toks[1]);
  if (toks[1] == "ERROR") throw NetworkError("Ident error: " + toks[2]);
  if (toks[1] != "USERID" || toks.size() != 4)
    throw NetworkError("Malformed ident response");
  
  os = util::TrimCopy(toks[2]);
  ident = util::TrimCopy(toks[3]);
}

} /* net namespace */
} /* util namespace */
