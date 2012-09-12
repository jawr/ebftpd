#include <cstdio>
#include <cstring>
#include <netdb.h>
#include <errno.h>
#include "resolver.hpp"
#include "exception.hpp"

namespace util
{

  resolver::resolver(socket_type_t socket_type, const std::string& host, uint16_t port) :
    _socket_type(socket_type),
    _host(host),
    _port(port),
    _results()
  {
    resolve();
  }

  resolver::resolver(socket_type_t socket_type) :
    _socket_type(socket_type),
    _host(),
    _port(-1),
    _results()
  {
  }

  void resolver::resolve()
  {
    struct addrinfo hints;
    struct addrinfo* res;
    char port_string[6];

    snprintf(port_string, sizeof(port_string), "%i", _port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = _socket_type;
    int error = getaddrinfo(_host.empty() ? NULL : _host.c_str(), port_string, &hints, &res);

    if (error)
    {
      if (error == EAI_NONAME)
      {
        throw util::unknown_host();
      }
      else if (error == EAI_SYSTEM)
      {
        util::errno_to_exception("getaddrinfo", errno);
      }
      else
      {
        throw util::unknown_network_error();
      }
    }
    else
    {
      struct addrinfo* cur = res;

      try
      {
        while (cur)
        {
          _results.push_back(endpoint(*cur->ai_addr, cur->ai_addrlen));
          cur = cur->ai_next;
        }
      }
      catch (...)
      {
        freeaddrinfo(res);
        throw;
      }

      freeaddrinfo(res);
    }
  }

  void resolver::resolve(const std::string& host, uint16_t port)
  {
    _host = host;
    _port = port;
    resolve();
  }

}
