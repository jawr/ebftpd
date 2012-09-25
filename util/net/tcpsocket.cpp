#include <cstdio>
#include <sys/socket.h>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include "util/net/tcpsocket.hpp"
#include "util/net/tcplistener.hpp"
#include "util/signalguard.hpp"

#include <iostream>

namespace util { namespace net
{

const TimePair TCPSocket::defaultTimeout = TimePair(60, 0);

TCPSocket::TCPSocket(const util::TimePair& timeout) :
  socket(-1),
  timeout(timeout),
  getcharBufferPos(0),
  getcharBufferLen(0)
  
{
}

TCPSocket::TCPSocket(const Endpoint& endpoint, const util::TimePair& timeout) :
  socket(-1),
  timeout(timeout),
  getcharBufferPos(0),
  getcharBufferLen(0)
{
  Connect(endpoint);
}

void TCPSocket::PopulateLocalEndpoint()
{
  struct sockaddr_storage localAddrStor;
  socklen_t localLen = sizeof(localAddrStor);
  struct sockaddr* localAddr = 
    static_cast<struct sockaddr*>(static_cast<void*>(&localAddrStor));
    
  if (getsockname(socket, (struct sockaddr*) localAddr, &localLen) < 0)
    throw NetworkSystemError(errno);

  localEndpoint = Endpoint(*localAddr, localLen);
}

void TCPSocket::PopulateRemoteEndpoint()
{
  struct sockaddr_storage remoteAddrStor;
  socklen_t remoteLen = sizeof(remoteAddrStor);
  struct sockaddr* remoteAddr = 
    static_cast<struct sockaddr*>(static_cast<void*>(&remoteAddrStor));

  if (getpeername(socket, (struct sockaddr*) remoteAddr, &remoteLen) < 0)
    throw NetworkSystemError(errno);
    
  remoteEndpoint = Endpoint(*remoteAddr, remoteLen);
}

void TCPSocket::Connect(const Endpoint& endpoint)
{
  socket = ::socket(endpoint.Family(), SOCK_STREAM, 0);
  if (socket < 0) throw NetworkSystemError(errno);

  SetTimeout();
  
  while (connect(socket, endpoint.Addr(), endpoint.Length()) < 0)
  {
    boost::this_thread::interruption_point();
    if (errno != EINTR)
    {
      int errno_ = errno;
      Close();
      if (errno_ == EWOULDBLOCK || errno_ == EAGAIN) errno_ = ETIMEDOUT;
      throw NetworkSystemError(errno_);
    }
  }
  
  boost::this_thread::interruption_point();
  PopulateRemoteEndpoint();
  PopulateLocalEndpoint();
}

void TCPSocket::Accept(TCPListener& listener)
{
  struct sockaddr_storage addrStor;
  socklen_t addrLen = sizeof(addrStor);
  struct sockaddr* addr =
    static_cast<struct sockaddr*>(static_cast<void*>(&addrStor));
    
  while ((socket = accept(listener.Socket(), addr, &addrLen)) < 0)
  {
    boost::this_thread::interruption_point();
    if (errno != EINTR)
    {
      if (errno == EWOULDBLOCK || errno == EAGAIN) errno = ETIMEDOUT;
      throw NetworkSystemError(errno);
    }
  }
    
  boost::this_thread::interruption_point();
  PopulateRemoteEndpoint();
  PopulateLocalEndpoint();
}

void TCPSocket::HandshakeTLS(TLSSocket::HandshakeRole role)
{
  tls.reset(new TLSSocket(*this, role));
}

size_t TCPSocket::Read(char* buffer, size_t bufferSize)
{
  if (tls.get()) return tls->Read(buffer, bufferSize);

  ssize_t result;
  while ((result = read(socket, buffer, bufferSize)) < 0)
  {
    boost::this_thread::interruption_point();
    if (errno != EINTR)
    {
      if (errno == EWOULDBLOCK || errno == EAGAIN) errno = ETIMEDOUT;
      throw NetworkSystemError(errno);
    }
  }

  boost::this_thread::interruption_point();
  if (!result) throw EndOfStream();
  
  return result;
}

void TCPSocket::Write(const char* buffer, size_t bufferLen)
{
  util::SignalGuard pipeGuard(SIGPIPE);
  
  if (tls.get())
  {
    tls->Write(buffer, bufferLen);
    return;
  }
  
  size_t written = 0;
  ssize_t result;
  while (bufferLen - written > 0)
  {
    while ((result = write(socket, buffer + written, bufferLen - written)) < 0)
    {
      boost::this_thread::interruption_point();
      if (errno != EINTR)
      {
        if (errno == EWOULDBLOCK || errno == EAGAIN) errno = ETIMEDOUT;
        throw NetworkSystemError(errno);
      }
    } 
    boost::this_thread::interruption_point();
    written += result;
  }
}

void TCPSocket::SetTimeout()
{
  if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout.Timeval(),
                 sizeof(timeout.Timeval())) < 0)
    throw NetworkSystemError(errno);
    
  if (setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, &timeout.Timeval(),
                 sizeof(timeout.Timeval())) < 0)
    throw NetworkSystemError(errno);
}

void TCPSocket::SetTimeout(const util::TimePair& timeout)
{
  this->timeout = timeout;
  SetTimeout();
}

char TCPSocket::GetcharBuffered()
{
  if (!getcharBufferLen)
  {
    getcharBufferLen = Read(getcharBuffer, sizeof(getcharBuffer));
    getcharBufferPos = getcharBuffer;
  }
  
  --getcharBufferLen;
  return *getcharBufferPos++;
}

void TCPSocket::Getline(char *buffer, size_t bufferSize, bool stripCRLF)
{
  if (!bufferSize) return;
  
  size_t bufferLen = 0;
  char ch;
  do
  {
    ch = GetcharBuffered();
    if (!stripCRLF || (ch != '\r' && ch != '\n'))
    {
      *buffer = ch;
      ++buffer;
      ++bufferLen;
    }
  }
  while (ch != '\n' && bufferLen < bufferSize - 1);
  
  *buffer = '\0';
  
  if (ch != '\n') throw BufferSizeExceeded();
}

void TCPSocket::Getline(std::string& buffer, bool stripCRLF)
{
  buffer.clear();
  buffer.reserve(defaultBufferSize);
  
  char ch;
  do
  {
    ch = GetcharBuffered();
    if (!stripCRLF || (ch != '\r' && ch != '\n'))
    {
      buffer += ch;
      if (buffer.length() == buffer.capacity())
        buffer.reserve((buffer.capacity() / defaultBufferSize) + 1);
    }
  }
  while (ch != '\n');
}

void TCPSocket::Close()
{
  boost::lock_guard<boost::mutex> lock(socketMutex);
  if (socket != -1)
  {
    if (tls.get()) 
    {
      tls->Close();
      delete tls.release();
    }
    close(socket);
    socket = -1;
  }
  
}

void TCPSocket::Shutdown()
{
  boost::lock_guard<boost::mutex> lock(socketMutex);
  if (socket != -1)  shutdown(socket, SHUT_RDWR);
}

 TCPSocket::State TCPSocket::WaitStateTimeout(State state, const util::TimePair* duration)
{
  fd_set readSet;
  FD_ZERO(&readSet);
  if (state.readable) FD_SET(socket, &readSet);
  
  fd_set writeSet;
  FD_ZERO(&writeSet);
  if (state.writeable) FD_SET(socket, &writeSet);
  
  struct timeval *tvPtr = 0;
  struct timeval tv;
  if (duration)
  {
    tv.tv_sec = duration->Seconds();
    tv.tv_usec = duration->Microseconds();
    tvPtr = &tv;
  }
  
  int result;
  while ((result = select(socket + 1, &readSet, &writeSet, NULL, tvPtr)) < 0)
  {
    boost::this_thread::interruption_point();
    if (errno != EINTR) throw NetworkSystemError(errno);
  }

  boost::this_thread::interruption_point();
  if (result)
  {
    if (!FD_ISSET(socket, &readSet) && state.readable) state.readable = false;
    if (!FD_ISSET(socket, &writeSet) && state.writeable) state.writeable = false;
  }
  
  return state;
}

 TCPSocket::State TCPSocket::WaitStateTimeout(State state, const util::TimePair& duration)
{
  return WaitStateTimeout(state, &duration);
}

 TCPSocket::State TCPSocket::WaitState(State state)
{
  return WaitStateTimeout(state, 0);
}

 TCPSocket::State TCPSocket::Pending(State state)
{
  util::TimePair duration(0, 0);
  return WaitStateTimeout(state, &duration);
}

} /* net namespace */
} /* util namespace */
