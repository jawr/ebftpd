#ifndef __NET_ENDPOINT_HPP
#define __NET_ENDPOINT_HPP

#include <string>
#include <netinet/in.h>
#include "ip.hpp"

namespace util
{

  class endpoint;

  namespace tcp
  {

    class server;
    class client;
    class resolver;

    namespace proxy
    {

      void relay(util::tcp::client&, const util::endpoint&, const std::string&, const std::string&);

    }
  }

  namespace udp
  {
    class resolver;
  }

  #pragma GCC diagnostic ignored "-Weffc++"
  
  class endpoint
  {
    friend class tcp::server;
    friend class tcp::client;
    friend class tcp::resolver;
    friend class udp::resolver;
    friend class resolver;
    friend void tcp::proxy::relay(util::tcp::client&, const util::endpoint&, const std::string&, const std::string&);
  private:
    socklen_t _addr_len;
    struct sockaddr_storage _addr;
    mutable std::string _ip;
    mutable std::string _hostname;
    mutable uint16_t _port;
    bool _empty;

    endpoint(const struct sockaddr& addr, socklen_t addr_len);

  public:
    endpoint() :
      _addr_len(sizeof(_addr)),
      _ip(),
      _hostname(),
      _port(0),
      _empty(true)
    {
    }

    endpoint(const std::string& _ip, uint16_t _port);

    uint16_t port() const;
    const std::string& ip() const;
    const std::string& hostname() const;
    bool empty() const
    {
      return _empty;
    }
  };

}

#endif
