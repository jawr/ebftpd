#ifndef __SSL_HPP
#define __SSL_HPP

#include <ctime>
#include <gnutls/gnutls.h>
#include <boost/noncopyable.hpp>

#if GNUTLS_VERSION_MAJOR < 3
#error "gnutls 3.0 or higher is required"
#endif

namespace util
{
  namespace tcp
  {
    class client;
  }

  namespace ssl
  {
    class client;

    enum handshake_type
    {
      do_accept,
      do_connect
    };
    
    struct session_data
    {
    friend class client;
      char* session;
      size_t size;
      
      session_data() : session(0), size(0) { }
      ~session_data() { delete [] session; }
    };

    class client : boost::noncopyable
    {
    private:
      gnutls_session_t ssl;
      int sock;
      time_t timeout;
      
      static const time_t default_timeout = 30;

      int select(int nfds, fd_set* readfds, fd_set* writefds,
                 fd_set* errorfds, struct timeval* timeout);

    public:
      client(time_t timeout = default_timeout);
      client(int sock, time_t timeout = default_timeout);
      client(util::tcp::client& tcp, time_t timeout = default_timeout);

      void negotiate(int sock, handshake_type type = do_connect, session_data* data = 0);
      void negotiate(int sock, time_t timeout, handshake_type type = do_connect, session_data* = 0);
      void negotiate(util::tcp::client& tcp, handshake_type type = do_connect, session_data* data = 0);
      void negotiate(util::tcp::client& tcp, time_t timeout, handshake_type type = do_connect, session_data* data = 0);

      size_t read(char* buffer, size_t buffer_size);
      size_t read(char* buffer, size_t buffer_size, time_t timeout);
      void write(const char* buffer, size_t buffer_len);
      void write(const char* buffer, size_t buffer_len, time_t timeout);
      void close();
    };

  }
}

#endif
