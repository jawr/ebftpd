#ifndef __UTIL_NET_TLS_HPP
#define __UTIL_NET_TLS_HPP

#include <memory>
#include <string>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

namespace util { namespace net
{

class TLSServerContext;
class TLSClientContext;

class TLSContext
{
protected:
  SSL_CTX* context;
  std::string certificate;
  std::string ciphers;
    
  static std::auto_ptr<TLSServerContext> server;
  static std::auto_ptr<TLSClientContext> client;
 
  TLSContext(const std::string& certificate,
             const std::string& ciphers);
  
  void Initialise();
  void InitialiseOpenSSL();
  virtual void CreateContext() = 0;
  void LoadCertificate();
  virtual void InitialiseSessionCaching() = 0;
  void SelectCiphers();
};

class TLSClientContext : public TLSContext
{
  TLSClientContext(const std::string& certificate,
                   const std::string& ciphers);

  void CreateContext();
  void InitialiseSessionCaching() { }
                   
public:
  static void Initialise(const std::string& certificate = "",
                         const std::string& ciphers = "");
  /* Throws TLSError, TLSProtocolError */

  static SSL_CTX* Get();
};

class TLSServerContext : public TLSContext
{
  TLSServerContext(const std::string& certificate,
                   const std::string& ciphers);

  void CreateContext();
  void InitialiseSessionCaching() { }
  
public:
  static void Initialise(const std::string& certificate,
                         const std::string& ciphers = "");
  /* Throws TLSError, TLSProtocolError */

  static SSL_CTX* Get();
};

} /* net namespace */
} /* util namespace */

#endif
