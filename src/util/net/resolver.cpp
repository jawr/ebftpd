#include <boost/lexical_cast.hpp>
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
  const char* charService = nullptr;
  if (port >= 0)
  {
    std::string service;
    try
    {
      service = boost::lexical_cast<std::string>(port);
    }
    catch (const boost::bad_lexical_cast&)
    {
      throw NetworkError("Invalid port number");
    }
    
    charService = service.c_str();
  }

  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = PF_UNSPEC;
  hints.ai_flags = AI_PASSIVE;
  hints.ai_socktype = static_cast<int>(socketType);

  const char* charHostname = nullptr;
  if (!hostname.empty()) charHostname = hostname.c_str();
  
  int error = getaddrinfo(charHostname, charService, &hints, &res);
  if (error)
  {
    if (error == EAI_SYSTEM) throw NetworkSystemError(errno);
    else throw ResolverError(error);
  }

  struct addrinfo* current = res;
  while (current)
  {
    results.emplace_back(Endpoint(*current->ai_addr, current->ai_addrlen));
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
  std::string hostname;
  hostname.resize(NI_MAXHOST);
  if (getnameinfo(ep.Addr(), ep.Length(), &hostname[0], hostname.size(), 
                  nullptr, 0, 0) != 0)
  {
    return ep.IP().ToString();
  }
  return hostname;
}

std::string ReverseResolve(const IPAddress& ip)
{
  return ReverseResolve(Endpoint(ip, -1));
}

} /* net namespace */
} /* util namespace */

#ifdef UTIL_NET_RESOLVER_TEST

#include <iostream>

int main()
{
  using namespace util::net;
  
  Resolver r(SockStream, "0.0.0.0", 21);
  for (Resolver::const_iterator it = r.begin();  it != r.end(); ++it)
  {
    std::cout << *it << std::endl;
  }
       
}

#endif
