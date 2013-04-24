//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <sys/socket.h>
#include <boost/thread/thread.hpp>
#include "util/net/tcpsocket.hpp"
#include "util/net/tcplistener.hpp"
#include "util/signalguard.hpp"
#include "util/scopeguard.hpp"

namespace
{
  util::SignalGuard pipeGuard(SIGPIPE);
}

namespace util { namespace net
{

namespace
{
  util::SignalGuard pipeGuard(SIGPIPE);
}

const TimePair TCPSocket::defaultTimeout = TimePair(60, 0);

TCPSocket::~TCPSocket()
{
  Close();
}

TCPSocket::TCPSocket(const util::TimePair& timeout) :
  socket(-1),
  timeout(timeout),
  getcharBufferPos(nullptr),
  getcharBufferLen(0)
  
{
}

TCPSocket::TCPSocket(const Endpoint& endpoint, const util::TimePair& timeout) :
  socket(-1),
  timeout(timeout),
  getcharBufferPos(nullptr),
  getcharBufferLen(0)
{
  Connect(endpoint);
}

void TCPSocket::PopulateLocalEndpoint(int socket)
{
  struct sockaddr_storage localAddrStor;
  socklen_t localLen = sizeof(localAddrStor);
  struct sockaddr* localAddr = reinterpret_cast<struct sockaddr*>(&localAddrStor);
    
  if (getsockname(socket, (struct sockaddr*) localAddr, &localLen) < 0)
    throw NetworkSystemError(errno);

  localEndpoint = Endpoint(*localAddr, localLen);
}

void TCPSocket::PopulateRemoteEndpoint(int socket)
{
  struct sockaddr_storage remoteAddrStor;
  socklen_t remoteLen = sizeof(remoteAddrStor);
  struct sockaddr* remoteAddr = reinterpret_cast<struct sockaddr*>(&remoteAddrStor);

  if (getpeername(socket, (struct sockaddr*) remoteAddr, &remoteLen) < 0)
    throw NetworkSystemError(errno);
    
  remoteEndpoint = Endpoint(*remoteAddr, remoteLen);
}

void TCPSocket::Connect(const Endpoint& remoteEndpoint, const Endpoint* localEndpoint)
{
  assert(socket < 0);
  int socket = ::socket(static_cast<int>(remoteEndpoint.Family()), SOCK_STREAM, 0);
  if (socket < 0) throw NetworkSystemError(errno);

  auto socketGuard = util::MakeScopeError([&socket]() { close(socket); }); (void) socketGuard;
  
  int optVal = 1;
  setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal));

  SetTimeout(socket);
  
  if (localEndpoint)
  {
    socklen_t addrLen = localEndpoint->Length();
    struct sockaddr_storage addrStor;
    memcpy(&addrStor, localEndpoint->Addr(), addrLen);
    struct sockaddr* addr = reinterpret_cast<struct sockaddr*>(&addrStor);

    if (bind(socket, addr, addrLen) < 0)
    {
      int errno_ = errno;
      throw util::net::NetworkSystemError(errno_);
    }
  }
  
  while (connect(socket, remoteEndpoint.Addr(), remoteEndpoint.Length()) < 0)
  {
    boost::this_thread::interruption_point();
    if (errno != EINTR)
    {
      int errno_ = errno;
      if (errno_ == EWOULDBLOCK || errno_ == EAGAIN || errno_ == ETIMEDOUT)
        throw TimeoutError();
      else
        throw NetworkSystemError(errno_);
    }
  }
  
  boost::this_thread::interruption_point();
  PopulateRemoteEndpoint(socket);
  PopulateLocalEndpoint(socket);
  
  std::lock_guard<std::mutex> lock(socketMutex);
  this->socket = socket;
}

void TCPSocket::Connect(const Endpoint& endpoint)
{
  Connect(endpoint, nullptr);
}

void TCPSocket::Connect(const Endpoint& remoteEndpoint, 
                        const Endpoint& localEndpoint)
{
  Connect(remoteEndpoint, &localEndpoint);
}

void TCPSocket::Accept(TCPListener& listener)
{
  struct sockaddr_storage addrStor;
  socklen_t addrLen = sizeof(addrStor);
  struct sockaddr* addr = reinterpret_cast<struct sockaddr*>(&addrStor);

  int socket;
  while ((socket = accept(listener.Socket(), addr, &addrLen)) < 0)
  {
    boost::this_thread::interruption_point();
    if (errno != EINTR)
    {
      if (errno == EWOULDBLOCK || errno == EAGAIN || errno == ETIMEDOUT)
        throw TimeoutError();
      else
        throw NetworkSystemError(errno);
    }
  }
  
  auto socketGuard = util::MakeScopeError([&socket]() { close(socket); }); (void) socketGuard;
  
  boost::this_thread::interruption_point();
  PopulateRemoteEndpoint(socket);
  PopulateLocalEndpoint(socket);
  
  std::lock_guard<std::mutex> lock(socketMutex);
  this->socket = socket;
}

void TCPSocket::HandshakeTLS(TLSSocket::HandshakeRole role, TCPSocket* reuse)
{
  tls.reset(new TLSSocket(*this, role, reuse && reuse->tls ? reuse->tls.get() : nullptr));
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
      if (errno == EWOULDBLOCK || errno == EAGAIN || errno == ETIMEDOUT)
        throw TimeoutError();
      else
        throw NetworkSystemError(errno);
    }
  }

  boost::this_thread::interruption_point();
  if (!result) throw EndOfStream();
  
  return result;
}

void TCPSocket::Write(const char* buffer, size_t bufferLen)
{
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
        if (errno == EWOULDBLOCK || errno == EAGAIN || errno == ETIMEDOUT)
          throw TimeoutError();
        else
          throw NetworkSystemError(errno);
      }
    } 
    boost::this_thread::interruption_point();
    written += result;
  }
}

void TCPSocket::SetTimeout(int socket)
{
  if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout.Timeval(), sizeof(timeout.Timeval())) < 0)
    throw NetworkSystemError(errno);
    
  if (setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, &timeout.Timeval(), sizeof(timeout.Timeval())) < 0)
    throw NetworkSystemError(errno);
}

void TCPSocket::SetTimeout(const util::TimePair& timeout)
{
  this->timeout = timeout;
  SetTimeout(socket);
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
  std::lock_guard<std::mutex> lock(socketMutex);
  if (socket >= 0)
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
  std::lock_guard<std::mutex> lock(socketMutex);
  if (socket >= 0)  shutdown(socket, SHUT_RDWR);
}

std::string TCPSocket::TLSCipher() const
{
  if (!tls.get()) return "NONE";
  return tls->Cipher();
}

} /* net namespace */
} /* util namespace */
