#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <csignal>
#include <cstring>
#include <cerrno>
#include <pthread.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <boost/thread.hpp>
#include "tcpclient.hpp"
#include "tcpserver.hpp"
#include "exception.hpp"
#include "signal.hpp"
#include "verify.hpp"

namespace util
{
  namespace tcp
  {
    client::client(time_t _timeout) :
      _timeout(_timeout),
      _ssl_negotiated(false),
      _getchar_buffer_len(0),
      _getchar_buffer_pos(NULL)
    {
    }

    client::client(int _socket, const util::endpoint& _endpoint, time_t _timeout) :
      _local_endpoint(_endpoint),
      _timeout(_timeout),
      _socket(_socket),
      _ssl_negotiated(false),
      _getchar_buffer_len(0),
      _getchar_buffer_pos(NULL)
    {
    }

    client::client(const util::endpoint& _endpoint, time_t _timeout) :
      _remote_endpoint(_endpoint),
      _timeout(_timeout),
      _ssl_negotiated(false),
      _getchar_buffer_len(0),
      _getchar_buffer_pos(NULL)
    {
      connect(_timeout);
    }

    client::~client()
    {
      close();
    }

    void client::connect(time_t _timeout)
    {
      _socket = ::socket(_remote_endpoint._addr.ss_family, SOCK_STREAM, 0);

      if (_socket < 0)
      {
        util::errno_to_exception("socket", errno);
      }

      if (!_bind_endpoint.empty() &&
          bind(_socket, (struct sockaddr*) &this->_bind_endpoint._addr,
               _bind_endpoint._addr_len) < 0)
      {
        int _errno = errno;
        close();
        util::errno_to_exception("bind", _errno);
      }

      if (_timeout < 0)
      {
        if (::connect(_socket, (struct sockaddr*) &this->_remote_endpoint._addr,
                      this->_remote_endpoint._addr_len) < 0)
        {
          int _errno = errno;
          close();
          util::errno_to_exception("connect", _errno);
        }
      }
      else
      {
        int flags = fcntl(_socket, F_GETFL);

        if (flags < 0 ||
            fcntl(_socket, F_SETFL, flags | O_NONBLOCK) < 0)
        {
          int _errno = errno;
          close();
          util::errno_to_exception("fcntl", _errno);
        }

        int result = ::connect(_socket, (struct sockaddr*) &this->_remote_endpoint._addr,
                               this->_remote_endpoint._addr_len);

        if (result < 0 &&
            errno != EINPROGRESS)
        {
          int _errno = errno;
          close();
          util::errno_to_exception("connect", _errno);
        }

        if (result)
        {
          fd_set read_set;
          fd_set write_set;
          struct timeval tv;
          FD_ZERO(&read_set);
          FD_SET(_socket, &read_set);
          FD_ZERO(&write_set);
          FD_SET(_socket, &write_set);
          tv.tv_sec = _timeout;
          tv.tv_usec = 0;

          result = select(_socket + 1, &read_set, &write_set, NULL, &tv);

          if (result < 0)
          {
            int _errno = errno;
            close();
            util::errno_to_exception("select", _errno);
          }

          if (!result)
          {
            close();
            util::errno_to_exception("select", ETIMEDOUT);
          }

          if (FD_ISSET(_socket, &read_set))
          {
            int error;
            socklen_t error_len = sizeof(error);

            if (getsockopt(_socket, SOL_SOCKET, SO_ERROR, &error, &error_len) < 0)
            {
              int _errno = errno;
              close();
              util::errno_to_exception("getsockopt", _errno);
            }

            if (error)
            {
              close();
              util::errno_to_exception("connect", error);
            }
          }
        }

        if (fcntl(_socket, F_SETFL, flags) < 0)
        {
          int _errno = errno;
          close();
          util::errno_to_exception("fcntl", _errno);
        }
      }

      if (getsockname(_socket, (struct sockaddr*) &this->_local_endpoint._addr,
                      &this->_local_endpoint._addr_len) < 0)
      {
        int _errno = errno;
        close();
        util::errno_to_exception("getsockname", _errno);
      }

      if (getpeername(_socket, (struct sockaddr*) &this->_remote_endpoint._addr,
                      &this->_remote_endpoint._addr_len) < 0)
      {
        int _errno = errno;
        close();
        util::errno_to_exception("getpeername", _errno);
      }

      int opt_val = 1;
      setsockopt(_socket, IPPROTO_TCP, TCP_NODELAY, &opt_val, sizeof(opt_val));
    }

    void client::connect(const util::endpoint& _endpoint, time_t _timeout)
    {
      this->_remote_endpoint = _endpoint;
      connect(_timeout);
    }

    void client::connect(const util::endpoint& _endpoint)
    {
      this->_remote_endpoint = _endpoint;
      connect(_timeout);
    }

    void client::negotiate_ssl(util::ssl::session_data* data)
    {
      _ssl_conn.negotiate(_socket, _timeout, data);
      _ssl_negotiated = true;
    }

    size_t client::read(char* buffer, size_t buffer_size, time_t _timeout)
    {
      if (_ssl_negotiated)
      {
        return _ssl_conn.read(buffer, buffer_size, _timeout);
      }


      fd_set read_set;
      FD_ZERO(&read_set);
      FD_SET(_socket, &read_set);

      int result;

      if (_timeout < 0)
      {
        result = select(_socket + 1,
                        &read_set, NULL, NULL, NULL);
      }
      else
      {
        struct timeval tv;
        tv.tv_sec = _timeout;
        tv.tv_usec = 0;

        result = select(_socket + 1,
                        &read_set, NULL, NULL, &tv);
      }

      if (result < 0)
      {
        util::errno_to_exception("select", errno);
      }

      if (!result)
      {
        util::errno_to_exception("select timeout", ETIMEDOUT);
      }

      ssize_t buffer_len = ::read(_socket, buffer, buffer_size);

      if (buffer_len < 0)
      {
        throw util::lost_connection();
      }
      else if (!buffer_len)
      {
        throw util::end_of_stream();
      }

      return buffer_len;
    }

    void client::write(const char* buffer, size_t buffer_len, time_t _timeout)
    {
      if (_ssl_negotiated)
      {
        _ssl_conn.write(buffer, buffer_len, _timeout);
        return;
      }

      util::signal::scope_block pipe_block(SIGPIPE);
      size_t buffer_written = 0;

      while (buffer_len - buffer_written > 0)
      {
        fd_set write_set;
        FD_ZERO(&write_set);
        FD_SET(_socket, &write_set);
        fd_set read_set;
        FD_ZERO(&read_set);

        int result;

        if (_timeout < 0)
        {
          result = select(_socket + 1,
                          &read_set, &write_set, NULL, NULL);
        }
        else
        {
          struct timeval tv;
          tv.tv_sec = _timeout;
          tv.tv_usec = 0;
          result = select(_socket + 1,
                          &read_set, &write_set, NULL, &tv);
        }

        if (result < 0)
        {
          util::errno_to_exception("select", errno);
        }

        if (!result)
        {
          util::errno_to_exception("select", ETIMEDOUT);
        }

        ssize_t bytes_written = ::write(_socket, buffer + buffer_written, buffer_len - buffer_written);
        if (bytes_written < 0)
        {
          throw util::lost_connection();
        }

        buffer_written += bytes_written;
      }
    }

    char client::getchar_buffered(time_t _timeout)
    {
      if (!_getchar_buffer_len)
      {
        _getchar_buffer_len = read(_getchar_buffer, sizeof(_getchar_buffer), _timeout);
        _getchar_buffer_pos = _getchar_buffer;
      }

      --_getchar_buffer_len;
      return *_getchar_buffer_pos++;
    }

    size_t client::getline(char* buffer, size_t buffer_size, time_t _timeout, bool strip_crlf)
    {
      char ch = '\0';
      size_t buffer_len = 0;

      while (ch != '\n' &&
             buffer_len < buffer_size - 1)
      {
        ch = getchar_buffered(_timeout);

        if (!strip_crlf ||
            (ch != '\r' && ch != '\n'))
        {
          *buffer++ = ch;
          ++buffer_len;
        }
      }

      *buffer = '\0';

      if (ch != '\n')
      {
        while (read(&ch, sizeof(ch), _timeout) > 0 &&
               ch != '\n')
        {
          // flushing remainder of line
        }
      }

      return buffer_len;
    }

    void client::close(bool linger)
    {
      if (_socket > 0)
      {
        if (linger)
        {
          struct linger opt_val;
          memset(&opt_val, 0, sizeof(opt_val));
          opt_val.l_onoff = 1;
          opt_val.l_linger = 1;
          setsockopt(_socket, SOL_SOCKET, SO_LINGER, &opt_val, sizeof(opt_val));
        }

        _ssl_conn.close();
        _socket.close();
        _ssl_negotiated = false;
      }
    }

    void client::shutdown(int how)
    {
      _socket.shutdown(how);
    }

    int client::select(int nfds, fd_set* readfds, fd_set* writefds,
                       fd_set* errorfds, struct timeval* timeout)
    {
      sigset_t set;
      sigemptyset(&set);
      sigaddset(&set, SIGPIPE);

      struct timespec ts;

      if (timeout)
      {
        ts.tv_sec = timeout->tv_sec;
        ts.tv_nsec = timeout->tv_usec * 1000;
      }

      int result = pselect(nfds, readfds, writefds, errorfds, timeout ? &ts : NULL, &set);

      if (result < 0 &&
          errno == EINTR)
      {
        boost::this_thread::interruption_point();
      }

      return result;
    }
  }
}
