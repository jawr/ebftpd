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

#ifndef __UTIL_NET_TLSSOCKET_HPP
#define __UTIL_NET_TLSSOCKET_HPP

#include <cstdint>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <boost/noncopyable.hpp>

namespace util { namespace net
{

class TCPSocket;

class TLSSocket : private boost::noncopyable
{
  SSL* session;

  void EvaluateResult(int result);  
  
public:
  enum HandshakeRole
  {
    Server,
    Client
  };
  
  ~TLSSocket();

  TLSSocket();
  /* No exceptions */
  
  TLSSocket(TCPSocket& socket, HandshakeRole role, TLSSocket* reuse = 0);
  /* Throws TLSError, TLSProtocolError, TLSSystemError, EndOfStream */
  
  void Handshake(TCPSocket& socket, HandshakeRole role, TLSSocket* reuse = 0);
  /* Throws TLSError, TLSProtocolError, TLSSystemError, EndOfStream */
  
  size_t Read(char* buffer, size_t bufferSize);
  /* Throws TLSError, TLSProtocolError, TLSSystemError, EndOfStream */
  void Write(const char* buffer, size_t bufferLen);
  /* Throws TLSError, TLSProtocolError, TLSSystemError, EndOfStream */
  
  void Close();
  /* No exceptions */
  
  std::string Cipher() const;
};

} /* net namespace */
} /* util namespace */

#endif
