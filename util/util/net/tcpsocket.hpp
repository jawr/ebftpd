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

#ifndef __UTIL_NET_TCPSOCKET_HPP
#define __UTIL_NET_TCPSOCKET_HPP

#include <memory>
#include <cstdio>
#include <sys/types.h>
#include <mutex>
#include <boost/noncopyable.hpp>
#include "util/timepair.hpp"
#include "util/net/endpoint.hpp"
#include "util/net/tlssocket.hpp"

namespace util { namespace net
{

class TCPListener;

class TCPSocket : boost::noncopyable
{
private:
  static const util::TimePair defaultTimeout; // 60 seconds
  static const size_t defaultBufferSize = BUFSIZ;

  int socket;
  std::mutex socketMutex;
  std::unique_ptr<TLSSocket> tls;
  util::TimePair timeout;
  Endpoint localEndpoint;
  Endpoint remoteEndpoint;

  char getcharBuffer[defaultBufferSize];
  char* getcharBufferPos;
  size_t getcharBufferLen;
  
  void Connect(const Endpoint& remoteEndpoint, 
               const Endpoint* localEndpoint);
  
  char GetcharBuffered();
  void SetTimeout(int socket);
  
  void PopulateLocalEndpoint(int socket);
  void PopulateRemoteEndpoint(int socket);

public:
  ~TCPSocket();

  TCPSocket(const util::TimePair& timeout = defaultTimeout);
  /* No exceptions */
  
  TCPSocket(const Endpoint& endpoint,
            const util::TimePair& timeout = defaultTimeout);
  /* Thwos NetworkSystemError */
  
  void Connect(const Endpoint& endpoint);
  /* Throws NetworkSystemError, InvalidIPAddressError */

  void Connect(const Endpoint& remoteEndpoint, const Endpoint& localEndpoint);
  /* Throws NetworkSystemError, InvalidIPAddressError */

  void Accept(TCPListener& listener);
  /* Throws NetworkSystemError, InvalidIPAddressError */
  
  void HandshakeTLS(TLSSocket::HandshakeRole role, TCPSocket* reuse = nullptr);
  /* Same as TLSSocket::Handshake() */
  
  size_t Read(char* buffer, size_t bufferSize);
  /* (No TLS) Throws NetworkSystemError, EndOfStream */
  /* (With TLS) Same as TLSSocket::Read() */
  
  void Write(const char* buffer, size_t bufferLen);
  /* (No TLS) Throws NetworkSystemError */
  /* (With TLS) Same as TLSSocket::Write() */
  
  void Getline(char* buffer, size_t bufferSize, bool stripCRLF = true);
  /* (No TLS) Throws NetworkSystemError, BufferSizeExceeded */
  /* (With TLS) Same as TLSSocket::Read(), BufferSizeExceeded */

  void Getline(std::string& buffer, bool stripCRLF);
  /* (No TLS) Throws NetworkSystemError */
  /* (With TLS) Same as TLSSocket::Read() */

  void SetTimeout(const util::TimePair& timeout);
  /* Throws NetworkSystemError */

  const util::TimePair& Timeout() const { return timeout; }
  
  void Close();
  /* No exceptions */
  
  void Shutdown();
  /* No exceptions */
  
  int Socket() const { return socket; }
  /* No exceptions */
  
  const Endpoint& RemoteEndpoint() const { return remoteEndpoint; }
  /* No exceptions */
  
  const Endpoint& LocalEndpoint() const { return localEndpoint; }
  /* No exceptions */
  
  bool IsConnected() const { return socket >= 0; }
  
  bool IsTLS() const { return tls.get() != 0; }
  std::string TLSCipher() const;
};

} /* net namespace */
} /* util namespace */

#endif
