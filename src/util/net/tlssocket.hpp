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
  
  TLSSocket(TCPSocket& socket, HandshakeRole role);
  /* Throws TLSError, TLSProtocolError, TLSSystemError, EndOfStream */
  
  void Handshake(TCPSocket& socket, HandshakeRole role);
  /* Throws TLSError, TLSProtocolError, TLSSystemError, EndOfStream */
  
  size_t Read(char* buffer, size_t bufferSize);
  /* Throws TLSError, TLSProtocolError, TLSSystemError, EndOfStream */
  void Write(const char* buffer, size_t bufferLen);
  /* Throws TLSError, TLSProtocolError, TLSSystemError, EndOfStream */
  
  void Close();
  /* No exceptions */
};

} /* net namespace */
} /* util namespace */

#endif
