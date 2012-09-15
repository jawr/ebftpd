#ifndef __TCP_SERVER_HPP
#define __TCP_SERVER_HPP

#include <boost/noncopyable.hpp>
#include "endpoint.hpp"

namespace util
{
  namespace tcp
  {
    class client;

    class server : boost::noncopyable
    {
    private:
      util::endpoint _endpoint;
      time_t _timeout;
      int _socket;

      server(const server& x);
      server& operator= (const server& x);

      void listen();

    public:
      server(time_t _timeout = -1);
      server(const util::endpoint& _endpoint, time_t _timeout = -1);

      void listen(const util::endpoint& _endpoint);
      void accept(client& _client)
      {
        accept(_client, _timeout);
      }
      void accept(client& _client, time_t _timeout);
      void close();

      int socket() const
      {
        return _socket;
      }
      util::endpoint& endpoint()
      {
        return _endpoint;
      }
    };
  }
}

#endif
