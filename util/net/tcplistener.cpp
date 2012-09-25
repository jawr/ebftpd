#include <algorithm>
#include <cerrno>
#include <sys/time.h>
#include <sys/select.h>
#include <boost/thread/thread.hpp>
#include "util/net/tcplistener.hpp"
#include "util/net/error.hpp"
#include "util/net/tcpsocket.hpp"

namespace util { namespace net
{

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
  std::cout << endpoint << std::endl;
  socket = ::socket(endpoint.Family(), SOCK_STREAM, 0);
  if (socket < 0) return throw NetworkSystemError(errno);

  int optVal = 1;
  setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal));
  
  socklen_t addrLen = endpoint.Length();
  struct sockaddr_storage addrStor;
  memcpy(&addrStor, endpoint.Addr(), addrLen);
  struct sockaddr* addr = 
    static_cast<struct sockaddr*>(static_cast<void*>(&addrStor));

  if (bind(socket, addr, addrLen) < 0)
  {
    std::cout << strerror(errno) << std::endl;
    int errno_ = errno;
    ::close(socket);
    socket = -1;
    throw util::net::NetworkSystemError(errno);
  }
  
  if (listen(socket, backlog) < 0)
  {
    int errno_ = errno;
    ::close(socket);
    socket = -1;
    throw util::net::NetworkSystemError(errno);
  }
  
  if (getsockname(socket, addr, &addrLen) < 0)
  {
    int errno_ = errno;
    ::close(socket);
    socket = -1;
    throw util::net::NetworkSystemError(errno);
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

bool TCPListener::WaitPendingTimeout(const TimePair* duration) const
{
  fd_set readSet;
  FD_ZERO(&readSet);
  FD_SET(socket, &readSet);
  FD_SET(interruptPipe[0], &readSet);
  int max = std::max(socket, interruptPipe[0]);
  
  struct timeval *tvPtr = 0;
  struct timeval tv;
  if (duration)
  {
    tv.tv_sec = duration->Seconds();
    tv.tv_usec = duration->Microseconds();
    tvPtr = &tv;
  }
  
  int result;
  while ((result = select(max + 1, &readSet, NULL, NULL, tvPtr)) < 0)
  {
    if (errno != EINTR) throw NetworkSystemError(errno);
    boost::this_thread::interruption_point();
  }

  if (!result) return false;
  
  if (FD_ISSET(interruptPipe[0], &readSet))
    boost::this_thread::interruption_point();
  
  if (FD_ISSET(socket, &readSet)) return true;
  
  return false;  
}

bool TCPListener::WaitPendingTimeout(const TimePair& duration) const
{
  return WaitPendingTimeout(&duration);
}

bool TCPListener::WaitPending() const
{
  return WaitPendingTimeout(0);
}

bool TCPListener::Pending() const
{
  TimePair duration(0, 0);
  return WaitPendingTimeout(&duration);
}


} /* net namespace */
} /* util namespace */
