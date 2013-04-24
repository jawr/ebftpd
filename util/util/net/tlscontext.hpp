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

#ifndef __UTIL_NET_TLS_HPP
#define __UTIL_NET_TLS_HPP

#include <memory>
#include <string>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <openssl/rsa.h>
#include <mutex>
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
  boost::shared_array<std::mutex> dummyMutexes;
    
  static std::unique_ptr<TLSServerContext> server;
  static std::unique_ptr<TLSClientContext> client;
  static boost::shared_array<std::mutex> mutexes;
  
  
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
  std::string contextId;

  TLSServerContext(const std::string& contextId,
                   const std::string& certificate,
                   const std::string& ciphers);

  void CreateContext();
  void InitialiseSessionCaching();
  void InitialiseDHKeyExchange();
  void DerivedInitialise()
  {
    InitialiseSessionCaching();
    InitialiseDHKeyExchange();
  }
  
public:
  static void Initialise(const std::string& contextId,
                         const std::string& certificate,
                         const std::string& ciphers = "");
  /* Throws TLSError, TLSProtocolError */

  static SSL_CTX* Get();
};

} /* net namespace */
} /* util namespace */

#endif
