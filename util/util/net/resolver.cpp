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

#include <netdb.h>
#include "util/net/resolver.hpp"
#include "util/scopeguard.hpp"
#include "util/error.hpp"

#include <iostream>

namespace util { namespace net
{

ResolverError::ResolverError(int code) :
  std::runtime_error(gai_strerror(code)),
  code(code)
{
}

Resolver::Resolver(SocketType socketType) :
  socketType(socketType)
{
}

Resolver::Resolver(SocketType socketType,
                   const std::string& hostname,
                   int32_t port) :
  socketType(socketType),
  hostname(hostname),
  port(port)
{
  Resolve();
}

Resolver::~Resolver()
{
  if (res) freeaddrinfo(res);
}

void Resolver::Resolve()
{
  std::string service;
  if (port >= 0) service = std::to_string(port);

  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = PF_UNSPEC;
  hints.ai_flags = AI_PASSIVE;
  hints.ai_socktype = static_cast<int>(socketType);

  int error = getaddrinfo(hostname.empty() ? nullptr : hostname.c_str(), 
                          service.empty() ? nullptr : service.c_str(), &hints, &res);
  if (error)
  {
    if (error == EAI_SYSTEM) throw NetworkSystemError(errno);
    else throw ResolverError(error);
  }

  struct addrinfo* current = res;
  while (current)
  {
    results.emplace_back(*current->ai_addr, current->ai_addrlen);
    current = current->ai_next;
  }

  freeaddrinfo(res);
  res = nullptr;
}

void Resolver::Resolve(const std::string& hostname, int32_t port)
{
  this->hostname = hostname;
  this->port = port;
  return Resolve();
}

std::string ReverseResolve(const Endpoint& ep)
{
  char hostname[NI_MAXHOST];
  if (getnameinfo(ep.Addr(), ep.Length(), hostname, sizeof(hostname), nullptr, 0, 0) != 0)
  {
    return ep.IP().ToString();
  }
  return std::string(hostname);
}

std::string ReverseResolve(const IPAddress& ip)
{
  return ReverseResolve(Endpoint(ip, -1));
}

} /* net namespace */
} /* util namespace */
