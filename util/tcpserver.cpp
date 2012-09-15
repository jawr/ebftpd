#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include "tcpserver.hpp"
#include "tcpclient.hpp"
#include "exception.hpp"

namespace util
{
  namespace tcp
  {

    server::server(time_t _timeout) :
      _timeout(_timeout),
      _socket(-1)
    {
    }

    server::server(const util::endpoint& _endpoint, time_t _timeout) :
      _endpoint(_endpoint),
      _timeout(_timeout),
      _socket(-1)
    {
      listen();
    }

    void server::listen()
    {
      _socket = ::socket(_endpoint._addr.ss_family, SOCK_STREAM, 0);

      if (_socket < 0)
      {
        util::errno_to_exception("socket", errno);
      }

      int opt_val = 1;
      setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));

      if (bind(_socket, (struct sockaddr*) &_endpoint._addr,
               _endpoint._addr_len) < 0)
      {
        int _errno = errno;

        ::close(_socket);
        _socket = -1;

        if (_errno == EADDRINUSE)
        {
          throw util::address_in_use();
        }
        else
        {
          util::errno_to_exception("bind", _errno);
        }
      }

      if (::listen(_socket, 10) < 0)
      {
        int _errno = errno;

        ::close(_socket);
        _socket = -1;

        util::errno_to_exception("listen", _errno);
      }

      if (getsockname(_socket, (struct sockaddr*) &_endpoint._addr,
                      &_endpoint._addr_len) < 0)
      {
        int _errno = errno;

        ::close(_socket);
        _socket = -1;

        util::errno_to_exception("getsockname", _errno);
      }
    }

    void server::listen(const util::endpoint& _endpoint)
    {
      this->_endpoint = _endpoint;
      listen();
    }

    void server::accept(client& _client, time_t _timeout)
    {
      if (_timeout >= 0)
      {
        fd_set read_set;
        struct timeval tv;
        int result;

        do
        {
          FD_ZERO(&read_set);
          FD_SET(_socket, &read_set);
          tv.tv_sec = _timeout;
          tv.tv_usec = 0;
        }
        while ((result = select(_socket + 1, &read_set, NULL, NULL, &tv)) < 0 &&
               errno == EINTR);

        if (result < 0)
        {
          util::errno_to_exception("select", errno);
        }

        if (!result ||
            !FD_ISSET(_socket, &read_set))
        {
          errno = ETIMEDOUT;
          throw util::timeout_error();
        }
      }

      _client._socket = ::accept(_socket, (struct sockaddr*)
                                 &_client._remote_endpoint._addr,
                                 &_client._remote_endpoint._addr_len);

      if (_client._socket < 0)
      {
        util::errno_to_exception("accept", errno);
      }

      if (getsockname(_client._socket, (struct sockaddr*)
                      &_client._local_endpoint._addr,
                      &_client._local_endpoint._addr_len) < 0)
      {
        util::errno_to_exception("getsockname", errno);
      }

      int opt_val = 1;
      setsockopt(_client._socket, IPPROTO_TCP, TCP_NODELAY,
                 &opt_val, sizeof(opt_val));
    }

    void server::close()
    {
      if (_socket != -1)
      {
        ::close(_socket);
        _socket = -1;
      }
    }

  }
}
