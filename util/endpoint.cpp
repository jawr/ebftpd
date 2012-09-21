#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include "endpoint.hpp"
#include "exception.hpp"
#include "resolver.hpp"

namespace util
{

  endpoint::endpoint(const struct sockaddr& addr, socklen_t addr_len) :
    _addr_len(addr_len),
    _ip(),
    _hostname(),
    _port(0),
    _empty(false)
  {
    memset(&_addr, 0, sizeof(_addr));
    memcpy(&_addr, &addr, addr_len);
    _addr_len = addr_len;
  }

  endpoint::endpoint(const std::string& _ip, uint16_t _port) :
    _addr_len(0),
    _ip(),
    _hostname(),
    _port(_port),
    _empty(false)
  {
    memset(&_addr, 0, sizeof(_addr));
    struct sockaddr* addr = (struct sockaddr*) &_addr;

    if (inet_pton(AF_INET, _ip.c_str(), & ((struct sockaddr_in*) addr)->sin_addr))
    {
      ((struct sockaddr_in*) addr)->sin_family = AF_INET;
      ((struct sockaddr_in*) addr)->sin_port = htons(_port);
      _addr_len = sizeof(struct sockaddr_in);
    }
    else if (inet_pton(AF_INET6, _ip.c_str(), & ((struct sockaddr_in6*) addr)->sin6_addr))
    {
      ((struct sockaddr_in6*) addr)->sin6_family = AF_INET6;
      ((struct sockaddr_in6*) addr)->sin6_port = htons(_port);
      _addr_len = sizeof(struct sockaddr_in6);
    }
    else
    {
      throw util::invalid_ip_address();
    }
  }

  uint16_t endpoint::port() const
  {
    if (!_port)
    {
      char port_cstr[6];
      int error = getnameinfo((struct sockaddr*) &_addr, _addr_len, NULL, 0,
                              port_cstr, sizeof(port_cstr), NI_NUMERICSERV);

      if (error)
      {
        if (error == EAI_SYSTEM)
        {
          util::errno_to_exception("getnameinfo", errno);
        }
        else
        {
          throw util::unknown_network_error();
        }
      }
      else
      {
        if (sscanf(port_cstr, "%hu", &_port) != 1)
        {
          throw util::unknown_network_error();
        }
      }
    }

    return _port;
  }

  const std::string& endpoint::ip() const
  {
    if (_ip.empty())
    {
      char ip_cstr[INET6_ADDRSTRLEN];
      int error = getnameinfo((struct sockaddr*) &_addr, _addr_len, ip_cstr, sizeof(ip_cstr),
                              NULL, 0, NI_NUMERICHOST);

      if (error)
      {
        if (error == EAI_SYSTEM)
        {
          util::errno_to_exception("getnameinfo", errno);
        }
        else
        {
          throw util::unknown_network_error();
        }
      }
      else
      {
        _ip = std::string(ip_cstr);
      }
    }

    return _ip;
  }

  const std::string& endpoint::hostname() const
  {
    if (_hostname.empty())
    {
      char hostname_cstr[NI_MAXHOST];
      int error = getnameinfo((struct sockaddr*) &_addr, _addr_len, hostname_cstr,
                              sizeof(hostname_cstr), NULL, 0, NI_NAMEREQD);

      if (error)
      {
        _hostname = ip();
      }
      else
      {
        _hostname = std::string(hostname_cstr);
      }
    }

    return _hostname;
  }

  std::ostream& operator<<(std::ostream& os, const endpoint& ep)
  {
    return (os << ep.ip() << ":" << ep.port());
  }
}
