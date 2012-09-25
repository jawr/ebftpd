#include <cstdio>
#include <iostream>
#include <cassert>
#include <csignal>
#include <cerrno>
#include <fcntl.h>
#include <boost/thread.hpp>
#include "ssl.hpp"
#include "tcpclient.hpp"
#include "exception.hpp"
#include "signal.hpp"

namespace util
{
  namespace ssl
  {
    class context
    {
      gnutls_certificate_credentials_t cred;

    public:
      context()
      {
        gnutls_global_init();
        gnutls_certificate_allocate_credentials(&cred);
        gnutls_certificate_set_x509_trust_file(cred, "bioftp4.pem", GNUTLS_X509_FMT_PEM);
      }

      const gnutls_certificate_credentials_t& credentials() const
      {
        return cred;
      }
    } ctx;

    client::client(time_t timeout) :
      sock(-1),
      timeout(timeout)
    {
    }

    client::client(int sock, time_t timeout) :
      sock(-1),
      timeout(timeout)
    {
      negotiate(sock);
    }

    client::client(util::tcp::client& tcp, time_t timeout) :
      sock(-1),
      timeout(timeout)
    {
      negotiate(tcp);
    }

    void client::negotiate(int sock, time_t timeout, handshake_type type, session_data* data)
    {
      assert(this->sock == -1);
      gnutls_init(&ssl, type == do_connect ? GNUTLS_CLIENT : GNUTLS_SERVER);
      gnutls_priority_set_direct(ssl, "PERFORMANCE:+COMP-NULL", NULL);
      gnutls_transport_set_ptr(ssl, (gnutls_transport_ptr_t) sock);
      
      if (data && data->session) gnutls_session_set_data(ssl, data->session, data->size);

      int flags = fcntl(sock, F_GETFL);
      if (flags < 0 || fcntl(sock, F_SETFL, flags | O_NONBLOCK))
      {
        util::errno_to_exception("fcntl", errno);
      }

      while (true)
      {
        int ret = gnutls_handshake(ssl);
        if (ret == GNUTLS_E_SUCCESS) break;
        else if (ret != GNUTLS_E_AGAIN)
        {
          gnutls_deinit(ssl);
          std::cout << gnutls_strerror(ret) << std::endl;
          throw util::unknown_network_error();
        }

        fd_set read_set;
        fd_set write_set;
        FD_ZERO(&read_set);
        FD_ZERO(&write_set);

        if (!gnutls_record_get_direction(ssl)) FD_SET(sock, &read_set);
        else FD_SET(sock, &write_set);

        struct timeval tv;
        tv.tv_sec = timeout;
        tv.tv_usec = 0;

        int result = select(sock + 1, &read_set, &write_set, NULL, &tv);
        if (result < 0) util::errno_to_exception("select", errno);
        else if (!result) util::errno_to_exception("select", ETIMEDOUT);
      }
      
      if (data  && (!data->session || !gnutls_session_is_resumed(ssl)))
      {
        delete [] data->session;
        data->session = 0;
        
        gnutls_session_get_data(ssl, NULL, &data->size);
        data->session = new char[data->size];
        gnutls_session_get_data(ssl, data->session, &data->size);
      }

      this->sock = sock;
    }

    void client::negotiate(int sock, handshake_type type, session_data* data)
    {
      negotiate(sock, timeout, type, data);
    }

    void client::negotiate(util::tcp::client& tcp, time_t timeout,
                           handshake_type type, session_data* data)
    {
      negotiate(tcp.socket(), timeout, type, data);
    }

    void client::negotiate(util::tcp::client& tcp, handshake_type type, session_data* data)
    {
      negotiate(tcp, timeout, type, data);
    }

    size_t client::read(char* buffer, size_t buffer_size)
    {
      return read(buffer, buffer_size, timeout);
    }

    size_t client::read(char* buffer, size_t buffer_size, time_t timeout)
    {
      assert(sock != -1);

      ssize_t buffer_len;
      while (true)
      {
        buffer_len = gnutls_record_recv(ssl, buffer, buffer_size);
        if (!buffer_len) throw util::end_of_stream();
        else if (buffer_len < 0)
        {
          if (buffer_len != GNUTLS_E_AGAIN) throw util::lost_connection();

          fd_set read_set;
          FD_ZERO(&read_set);
          FD_SET(sock, &read_set);

          struct timeval tv;
          tv.tv_sec = timeout;
          tv.tv_usec = 0;

          int result = select(sock + 1, &read_set, NULL, NULL, timeout < 0 ? NULL : &tv);
          if (result < 0) util::errno_to_exception("select", errno);
          if (!result) util::errno_to_exception("select timeout", ETIMEDOUT);
        }
        else
        {
          break;
        }
      }

      return buffer_len;
    }

    void client::write(const char* buffer, size_t buffer_len)
    {
      write(buffer, buffer_len, timeout);
    }

    void client::write(const char* buffer, size_t buffer_len, time_t timeout)
    {
      assert(sock != -1);

      util::signal::scope_block pipe_block(SIGPIPE);
      size_t buffer_written = 0;
      while (buffer_len - buffer_written > 0)
      {
        ssize_t bytes_written = gnutls_record_send(ssl, buffer + buffer_written,
                                                   buffer_len - buffer_written);

        if (bytes_written <= 0)
        {
          if (bytes_written != GNUTLS_E_AGAIN) throw util::lost_connection();

          fd_set write_set;
          FD_ZERO(&write_set);
          FD_SET(sock, &write_set);

          struct timeval tv;
          tv.tv_sec = timeout;
          tv.tv_usec = 0;

          int result = select(sock + 1, NULL, &write_set, NULL, timeout < 0 ? NULL : &tv);
          if (result < 0) util::errno_to_exception("select", errno);
          if (!result) util::errno_to_exception("select", ETIMEDOUT);
        }
        else
        {
          buffer_written += bytes_written;
        }
      }
    }

    void client::close()
    {
      if (sock != -1)
      {
        // gnutls_bye is way too slow!
        gnutls_deinit(ssl);
        sock = -1;
      }
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
      if (result < 0 && errno == EINTR) boost::this_thread::interruption_point();

      return result;
    }
  }
}
