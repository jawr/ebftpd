#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <boost/bind.hpp>
#include "util/net/tlscontext.hpp"
#include "util/net/tlserror.hpp"

namespace util { namespace net
{

std::auto_ptr<TLSClientContext> TLSContext::client;
std::auto_ptr<TLSServerContext> TLSContext::server;
boost::mutex* TLSContext::mutexes = new boost::mutex[CRYPTO_num_locks()];

TLSContext::~TLSContext()
{
  if (context) SSL_CTX_free(context);
  delete [] mutexes;
}

TLSContext::TLSContext(const std::string& certificate, const std::string& ciphers) :
  context(0),
  certificate(certificate),
  ciphers(ciphers)
{
}

void TLSContext::Initialise()
{
  InitialiseThreadSafety();
  InitialiseOpenSSL();
  CreateContext();
  LoadCertificate();
  InitialiseSessionCaching();
  SelectCiphers();
}
void TLSContext::InitialiseThreadSafety()
{
  CRYPTO_set_id_callback(ThreadIdCallback);
  CRYPTO_set_locking_callback(MutexLockCallback);
}

void TLSContext::InitialiseOpenSSL()
{
  if (client.get() && server.get()) return;
  
  SSL_library_init();
  SSL_load_error_strings();
  OpenSSL_add_all_algorithms();    

  unsigned int randSeed;
  while (!RAND_status())
  {
    randSeed = rand(); // replace with better seed?
    RAND_seed(&randSeed, sizeof(randSeed));
  }
}

void TLSContext::LoadCertificate()
{
  if (!certificate.empty())
  {
    if (SSL_CTX_use_certificate_chain_file(context, certificate.c_str()) != 1 ||
        SSL_CTX_use_PrivateKey_file(
          context, certificate.c_str(), SSL_FILETYPE_PEM) != 1 ||
        SSL_CTX_check_private_key(context) != 1) throw TLSProtocolError();
  }
  
  SSL_CTX_set_tmp_rsa_callback(context, TempRSACallback);
}

RSA* TLSContext::GenerateRSA(int keyLength)
{
  return RSA_generate_key(keyLength, RSA_F4, NULL, NULL);
}

RSA* TLSContext::TempRSACallback(SSL* session, int isExport, int keyLength)
{
  if (isExport || keyLength >= 1024) return GenerateRSA(1024);
  else return GenerateRSA(512);
}

void TLSContext::SelectCiphers()
{
 if (!ciphers.empty() && SSL_CTX_set_cipher_list(context, ciphers.c_str()) != 1)
    throw TLSError("No valid ciphers selected");
}

unsigned long TLSContext::ThreadIdCallback()
{
  return pthread_self();
}

void TLSContext::MutexLockCallback(int mode, int n, const char* file, int line)
{
  if (mode & CRYPTO_LOCK) mutexes[n].lock();
  else mutexes[n].unlock();
  
  (void) file;
  (void) line;
}

TLSClientContext::TLSClientContext(const std::string& certificate,
                                   const std::string& ciphers) :
  TLSContext(certificate, ciphers)
{
}

void TLSClientContext::CreateContext()
{
  context = SSL_CTX_new(TLSv1_client_method());
  if (!context) throw TLSProtocolError();
  SSL_CTX_set_options(context, SSL_OP_ALL);
}

void TLSClientContext::Initialise(const std::string& certificate,
                                  const std::string& ciphers)
{
  assert(!client.get());
  client.reset(new TLSClientContext(certificate, ciphers));
  try
  {
    client->TLSContext::Initialise();
  }
  catch (...)
  {
    delete client.release();
    throw;
  }
}

SSL_CTX* TLSClientContext::Get()
{
  assert(client.get());
  return client->context;
}

TLSServerContext::TLSServerContext(const std::string& certificate,
                                   const std::string& ciphers) :
  TLSContext(certificate, ciphers)
{
  
}

void TLSServerContext::CreateContext()
{
  context = SSL_CTX_new(TLSv1_server_method());
  if (!context) throw TLSProtocolError();

  unsigned long options = SSL_OP_NO_SSLv2 | SSL_OP_ALL;
#if (OPENSSL_VERSION_NUMBER >= 0x10000000)
  options |= SSL_OP_NO_COMPRESSION;
#endif  

  SSL_CTX_set_options(context, options);
}

void TLSServerContext::Initialise(const std::string& certificate,
                                  const std::string& ciphers)
{
  assert(!server.get());
  server.reset(new TLSServerContext(certificate, ciphers));
  try
  {
    server->TLSContext::Initialise();
  }
  catch (...)
  {
    delete server.release();
    throw;
  }
}

SSL_CTX* TLSServerContext::Get()
{
  assert(server.get());
  return server->context;
}

} /* net namespace */
} /* util namespace */


#ifdef __UTIL_NET_TLSCONTEXT_TEST

using namespace util::net;

int main()
{
  TLSClientContext::Initialise("/glftpd/ftpd-dsa.pem");
  TLSClientContext::Get();
}

#endif
