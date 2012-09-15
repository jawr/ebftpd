#ifndef __NET_EXCEPTION_HPP
#define __NET_EXCEPTION_HPP

#include <stdexcept>

namespace util
{

  void errno_to_exception(const char* function, int _errno);

  class interrupted : public std::runtime_error
  {
  public:
    interrupted() :
      std::runtime_error("interrupted")
    {
    }
  };

  class network_error : public std::runtime_error
  {
  public:
    network_error() :
      std::runtime_error("no error message")
    {
    }

    network_error(const char* message) :
      std::runtime_error(message)
    {
    }
  };

  class address_in_use : public network_error
  {
  public:
    address_in_use() :
      network_error("address already in use")
    {
    }
  };

  class end_of_stream : public network_error
  {
  public:
    end_of_stream() :
      network_error("end of stream")
    {
    }
  };

  class lost_connection : public network_error
  {
  public:
    lost_connection() :
      network_error("lost connection")
    {
    }
  };

  class timeout_error : public network_error
  {
  public:
    timeout_error() :
      network_error("timed out")
    {
    }
  };

  class already_listening : public network_error
  {
  public:
    already_listening() :
      network_error("already listening")
    {
    }
  };

  class ssl_already_negotiated : public network_error
  {
  public:
    ssl_already_negotiated() :
      network_error("ssl already negotiated")
    {
    }
  };

  class already_connected : public network_error
  {
  public:
    already_connected() :
      network_error("already connected")
    {
    }
  };

  class connection_refused : public network_error
  {
  public:
    connection_refused() :
      network_error("connection refused")
    {
    }
  };

  class unknown_host : public network_error
  {
  public:
    unknown_host() :
      network_error("unknown host")
    {
    }
  };

  class invalid_ip_address : public network_error
  {
  public:
    invalid_ip_address() :
      network_error("invalid ip address")
    {
    }
  };
  
  class ssl_handshake_error : public network_error
  {
  public:
    ssl_handshake_error() :
      network_error("ssl handshake error")
    {
    }
  };
  
  class unknown_ssl_error : public network_error
  {
  public:
    unknown_ssl_error() :
      network_error("unknown ssl error")
    {
    }
  };

  class unknown_network_error : public network_error
  {
  public:
    unknown_network_error() :
      network_error("unknown network error")
    {
    }

    unknown_network_error(const char* message) :
      network_error(message)
    {
    }
  };

  namespace proxy
  {

    class method_error : public network_error
    {
    public:
      method_error() :
        network_error("method error")
      {
      }
    };

    class not_allowed_error : public network_error
    {
    public:
      not_allowed_error() :
        network_error("not allowed error")
      {
      }
    };

    class authentication_error : public network_error
    {
    public:
      authentication_error() :
        network_error("authentication error")
      {
      }
    };

  }
}

#endif
