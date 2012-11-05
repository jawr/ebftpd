#ifndef __UTIL_NET_TLS_HPP
#define __UTIL_NET_TLS_HPP

#include <memory>
#include <string>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <openssl/rsa.h>
#include <boost/thread/mutex.hpp>
#include <boost/shared_array.hpp>

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
  
   // dummy prevents the static mutexes 
   // going out of scope before the contexts do
   // a nicer solution for this would be nice
  boost::shared_array<boost::mutex> dummy;
    
  static std::unique_ptr<TLSServerContext> server;
  static std::unique_ptr<TLSClientContext> client;
  static boost::shared_array<boost::mutex> mutexes;
  
  
  virtual ~TLSContext();
  
  TLSContext(const std::string& certificate,
             const std::string& ciphers);
  
  void Initialise();
  void InitialiseOpenSSL();
  virtual void CreateContext() = 0;
  void LoadCertificate();
  void SelectCiphers();
  void InitialiseThreadSafety();
  virtual void DerivedInitialise() = 0;
  
  static unsigned long ThreadIdCallback();
  static void MutexLockCallback(int mode, int n, const char * file, int line);
};

class TLSClientContext : public TLSContext
{
  TLSClientContext(const std::string& certificate,
                   const std::string& ciphers);

  void CreateContext();
  void InitialiseSessionCaching() { }
  void DerivedInitialise() { }
                   
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
  void InitialiseDHKeyExchange();
  void DerivedInitialise()
  {
    InitialiseSessionCaching();
    InitialiseDHKeyExchange();
  }
  
public:
  static void Initialise(const std::string& certificate,
                         const std::string& ciphers = "");
  /* Throws TLSError, TLSProtocolError */

  static SSL_CTX* Get();
};

} /* net namespace */
} /* util namespace */

#endif
