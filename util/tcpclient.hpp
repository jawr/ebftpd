#ifndef __TCP_CLIENT_HPP
#define __TCP_CLIENT_HPP

#include <cstdio>
#include <pthread.h>
#include <sys/select.h>
#include <boost/noncopyable.hpp>
#include "endpoint.hpp"
#include "ssl.hpp"
#include "descriptor.hpp"

namespace util
{
  namespace tcp
  {
    class client : boost::noncopyable
    {
      friend class server;
      friend class util::ssl::client;
    private:

      util::endpoint _local_endpoint;
      util::endpoint _remote_endpoint;
      time_t _timeout;
      util::descriptor _socket;
      util::ssl::client _ssl_conn;

      bool _ssl_negotiated;
      char _getchar_buffer[BUFSIZ];
      size_t _getchar_buffer_len;
      char* _getchar_buffer_pos;

      util::endpoint _bind_endpoint;

      static const time_t default_timeout = 30;

      client(const client& x);
      client& operator= (const client& x);

      void connect(time_t _timeout = default_timeout);
      char getchar_buffered(time_t _timeout);
      void initialise();
      int select(int nfds, fd_set* readfds, fd_set* writefds,
                 fd_set* errorfds, struct timeval* timeout);

    public:
      client(time_t _timeout = default_timeout);
      client(int _socket, const util::endpoint& _endpoint, time_t _timeout = default_timeout);
      client(const util::endpoint& _endpoint, time_t _timeout = default_timeout);
      ~client();

      void connect(const util::endpoint& _endpoint, time_t _timeout);
      void connect(const util::endpoint& _endpoint);

      void negotiate_ssl(util::ssl::session_data* data = 0);

      size_t read(char* buffer, size_t buffer_size, time_t _timeout);
      size_t read(char* buffer, size_t buffer_size)
      {
        return read(buffer, buffer_size, _timeout);
      }

      void write(const char* buffer, size_t buffer_len, time_t _timeout);
      void write(const char* buffer, size_t buffer_len)
      {
        write(buffer, buffer_len, _timeout);
      }

      size_t getline(char* buffer, size_t buffer_size, time_t _timeout, bool strip_crlf = true);
      size_t getline(char* buffer, size_t buffer_size, bool strip_crlf = true)
      {
        return getline(buffer, buffer_size, _timeout, strip_crlf);
      }

      void close(bool linger = false);
      void shutdown(int how);

      util::endpoint& local_endpoint()
      {
        return _local_endpoint;
      }
      util::endpoint& remote_endpoint()
      {
        return _remote_endpoint;
      }
      int socket()
      {
        return _socket;
      }
      util::ssl::client& ssl_conn()
      {
        return _ssl_conn;
      }
      bool ssl_negotiated() const
      {
        return _ssl_negotiated;
      }
      bool connected() const
      {
        return _socket.is_open();
      }

      const util::endpoint& bind_endpoint() const
      {
        return _bind_endpoint;
      }
      void set_bind_endpoint(const util::endpoint& bind_endpoint)
      {
        this->_bind_endpoint = bind_endpoint;
      }
    };

  }
}

#endif
