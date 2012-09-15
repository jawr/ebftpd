#ifndef __RESOLVER_HPP
#define __RESOLVER_HPP

#include <string>
#include <vector>
#include <sys/socket.h>
#include <boost/noncopyable.hpp>
#include "endpoint.hpp"

namespace util
{
  namespace socket_types
  {
    enum socket_type
    {
      stream = SOCK_STREAM,
      datagram = SOCK_DGRAM
    };
  }

  typedef socket_types::socket_type socket_type_t;
  typedef std::vector<endpoint> results_t;

  class resolver : boost::noncopyable
  {
  private:
    socket_type_t _socket_type;
    std::string _host;
    uint16_t _port;
    std::vector<endpoint> _results;

    void resolve();

  public:
    resolver(socket_type_t socket_type,
             const std::string& host,
             uint16_t port = 0);
    resolver(socket_type_t socket_type);
    
    virtual ~resolver() { }

    void resolve(const std::string& host, uint16_t port);

    const std::string& host() const { return _host; }
    uint16_t port() const { return _port; }
    results_t& results() { return _results; }

    typedef results_t::iterator results_iterator;
    typedef results_t::const_iterator const_results_iterator;
  };

  namespace tcp
  {

    class resolver : public util::resolver
    {
    public:
      resolver(const std::string& host, uint16_t port = 0) :
        util::resolver(socket_types::stream, host, port)
      {
      }

      resolver() :
        util::resolver(socket_types::stream)
      {
      }
    };

  }

  namespace udp
  {

    class resolver : public util::resolver
    {
    public:
      resolver(const std::string& host, uint16_t port = 0) :
        util::resolver(socket_types::datagram, host, port)
      {
      }

      resolver() :
        util::resolver(socket_types::datagram)
      {
      }
    };
  }
}

#endif
