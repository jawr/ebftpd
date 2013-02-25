#include <algorithm>
#include <cerrno>
#include <sys/time.h>
#include <boost/thread/thread.hpp>
#include "util/net/tcplistener.hpp"
#include "util/net/error.hpp"
#include "util/net/tcpsocket.hpp"

namespace util { namespace net
{

TCPListener::~TCPListener()
{
  Close();
}

TCPListener::TCPListener(const util::net::Endpoint& endpoint, int backlog) :
  endpoint(endpoint),
  socket(-1),
  backlog(backlog)
{
  Listen();
}

TCPListener::TCPListener(int backlog) :
  socket(-1),
  backlog(backlog)
{
}

void TCPListener::Listen()
{
  socket = ::socket(static_cast<int>(endpoint.Family()), SOCK_STREAM, 0);
  if (socket < 0) return throw NetworkSystemError(errno);

  int optVal = 1;
  setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal));
  
  socklen_t addrLen = endpoint.Length();
  struct sockaddr_storage addrStor;
  memcpy(&addrStor, endpoint.Addr(), addrLen);
  struct sockaddr* addr = 
    reinterpret_cast<struct sockaddr*>(&addrStor);

  if (bind(socket, addr, addrLen) < 0)
  {
    int errno_ = errno;
    ::close(socket);
    socket = -1;
    throw util::net::NetworkSystemError(errno_);
  }
  
  if (listen(socket, backlog) < 0)
  {
    int errno_ = errno;
    ::close(socket);
    socket = -1;
    throw util::net::NetworkSystemError(errno_);
  }
  
  if (getsockname(socket, addr, &addrLen) < 0)
  {
    int errno_ = errno;
    ::close(socket);
    socket = -1;
    throw util::net::NetworkSystemError(errno_);
  }
  
  endpoint = util::net::Endpoint(*addr, addrLen);
}

void TCPListener::Listen(const util::net::Endpoint& endpoint)
{
  this->endpoint = endpoint;
  Listen();
}

void TCPListener::Accept(TCPSocket& socket)
{
  socket.Accept(*this);
}

void TCPListener::Close()
{
  if (socket >= 0)
  {
    close(socket);
    socket = -1;
  }
}

} /* net namespace */
} /* util namespace */
