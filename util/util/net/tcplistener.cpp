#include <algorithm>
#include <cerrno>
#include <sys/time.h>
#include <boost/thread/thread.hpp>
#include "util/net/tcplistener.hpp"
#include "util/net/error.hpp"
#include "util/net/tcpsocket.hpp"
#include "util/scopeguard.hpp"

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
  assert(socket == -1);
  int socket = ::socket(static_cast<int>(endpoint.Family()), SOCK_STREAM, 0);
  if (socket < 0) return throw NetworkSystemError(errno);

  auto socketGuard = util::MakeScopeError([&socket]() {  close(socket);  }); (void) socketGuard;
  
  int optVal = 1;
  setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal));
  
  socklen_t addrLen = endpoint.Length();
  struct sockaddr_storage addrStor;
  memcpy(&addrStor, endpoint.Addr(), addrLen);
  struct sockaddr* addr = reinterpret_cast<struct sockaddr*>(&addrStor);

  if (bind(socket, addr, addrLen) < 0)
  {
    int errno_ = errno;
    throw util::net::NetworkSystemError(errno_);
  }
  
  if (listen(socket, backlog) < 0)
  {
    int errno_ = errno;
    throw util::net::NetworkSystemError(errno_);
  }
  
  if (getsockname(socket, addr, &addrLen) < 0)
  {
    int errno_ = errno;
    throw util::net::NetworkSystemError(errno_);
  }
  
  endpoint = util::net::Endpoint(*addr, addrLen);
  
  std::lock_guard<std::mutex> lock(socketMutex);
  this->socket = socket;
}

void TCPListener::Listen(const util::net::Endpoint& endpoint)
{
  this->endpoint = endpoint;
  Listen();
}

void TCPListener::Accept(TCPSocket& socket)
{
  assert(this->socket >= 0);
  socket.Accept(*this);
}

void TCPListener::Shutdown()
{
  std::lock_guard<std::mutex> lock(socketMutex);
  if (socket >= 0)  shutdown(socket, SHUT_RDWR);
}

void TCPListener::Close()
{
  std::lock_guard<std::mutex> lock(socketMutex);
  if (socket >= 0)
  {
    close(socket);
    socket = -1;
  }
}

} /* net namespace */
} /* util namespace */
