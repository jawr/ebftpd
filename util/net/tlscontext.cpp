#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <boost/bind.hpp>
#include "util/net/tlscontext.hpp"
#include "util/net/tlserror.hpp"

// some of this code is based loosely on code ftom pure-ftpd's tls.c
// which seems to have parts based on openssl's s_server.c

namespace util { namespace net
{

std::auto_ptr<TLSClientContext> TLSContext::client;
std::auto_ptr<TLSServerContext> TLSContext::server;
boost::mutex* TLSContext::mutexes = new boost::mutex[CRYPTO_num_locks()];

namespace
{

RSA* GenerateRSA(int keyLength)
{
  return RSA_generate_key(keyLength, RSA_F4, NULL, NULL);
}

RSA* TempRSACallback(SSL* session, int isExport, int keyLength)
{
  if (isExport || keyLength >= 1024) return GenerateRSA(1024);
  else return GenerateRSA(512);
  (void) session;
}

DH *GenerateDH512()
{
  static unsigned char dh512g[] = { 0x02, };
  static unsigned char dh512p[] = {
    0xF5,0x2A,0xFF,0x3C,0xE1,0xB1,0x29,0x40,0x18,0x11,0x8D,0x7C,
    0x84,0xA7,0x0A,0x72,0xD6,0x86,0xC4,0x03,0x19,0xC8,0x07,0x29,
    0x7A,0xCA,0x95,0x0C,0xD9,0x96,0x9F,0xAB,0xD0,0x0A,0x50,0x9B,
    0x02,0x46,0xD3,0x08,0x3D,0x66,0xA4,0x5D,0x41,0x9F,0x9C,0x7C,
    0xBD,0x89,0x4B,0x22,0x19,0x26,0xBA,0xAB,0xA2,0x5E,0xC3,0x55,
    0xE9,0x2A,0x05,0x5F,
  };
  
  DH *dh =DH_new();
  if (!dh) return NULL;
  dh->p = BN_bin2bn(dh512p, sizeof(dh512p), NULL);
  dh->g = BN_bin2bn(dh512g, sizeof(dh512g), NULL);
  if (!dh->p || !dh->g)
  {
    DH_free(dh);
    return NULL;
  }
  return dh;
}

DH *GenerateDH1024()
{
  static unsigned char dh1024g[] = { 0x02, };
  static unsigned char dh1024p[] = 
  {
    0xF4,0x88,0xFD,0x58,0x4E,0x49,0xDB,0xCD,0x20,0xB4,0x9D,0xE4,
    0x91,0x07,0x36,0x6B,0x33,0x6C,0x38,0x0D,0x45,0x1D,0x0F,0x7C,
    0x88,0xB3,0x1C,0x7C,0x5B,0x2D,0x8E,0xF6,0xF3,0xC9,0x23,0xC0,
    0x43,0xF0,0xA5,0x5B,0x18,0x8D,0x8E,0xBB,0x55,0x8C,0xB8,0x5D,
    0x38,0xD3,0x34,0xFD,0x7C,0x17,0x57,0x43,0xA3,0x1D,0x18,0x6C,
    0xDE,0x33,0x21,0x2C,0xB5,0x2A,0xFF,0x3C,0xE1,0xB1,0x29,0x40,
    0x18,0x11,0x8D,0x7C,0x84,0xA7,0x0A,0x72,0xD6,0x86,0xC4,0x03,
    0x19,0xC8,0x07,0x29,0x7A,0xCA,0x95,0x0C,0xD9,0x96,0x9F,0xAB,
    0xD0,0x0A,0x50,0x9B,0x02,0x46,0xD3,0x08,0x3D,0x66,0xA4,0x5D,
    0x41,0x9F,0x9C,0x7C,0xBD,0x89,0x4B,0x22,0x19,0x26,0xBA,0xAB,
    0xA2,0x5E,0xC3,0x55,0xE9,0x2F,0x78,0xC7,
  };

  DH *dh;
  dh = DH_new();
  if (!dh) return NULL;
  dh->p = BN_bin2bn(dh1024p, sizeof(dh1024p), NULL);
  dh->g = BN_bin2bn(dh1024g, sizeof(dh1024g), NULL);
  if (!dh->p || !dh->g)
  {
    DH_free(dh);
    return NULL;
  }
  
  return dh;
}

DH *TempDHCallback(SSL* session, int isExport, int keyLength)
{
  if (isExport == 0 || keyLength >= 1024) return GenerateDH1024();
  else return GenerateDH512();
  (void) session;
}

}

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
  SelectCiphers();
  DerivedInitialise();
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

void TLSServerContext::InitialiseDHKeyExchange()
{
  // if this fails, ciphers requiring DH key exchange
  // will not work
  bool failed = false;
  DH* dh = 0;
  
  BIO *bio = BIO_new_file(certificate.c_str(), "r");
  if (!bio)
  {
    failed = true;
    goto finish;
  }

#if OPENSSL_VERSION_NUMBER >= 0x00904000L
  dh = PEM_read_bio_DHparams(bio, NULL, NULL, NULL);
#else
  dh = PEM_read_bio_DHparams(bio, NULL, NULL);
#endif

  if (!dh)
  {
    failed = true;
    goto finish;
  }
  
  if (SSL_CTX_set_tmp_dh(context, dh) != 1)
  {
    failed = true;
    goto finish;
  }
  
finish:
  if (dh) DH_free(dh);
  if (bio) BIO_free(bio);
  if (failed) SSL_CTX_set_tmp_dh_callback(context, TempDHCallback);
}

SSL_CTX* TLSServerContext::Get()
{
  assert(server.get());
  return server->context;
}

} /* net namespace */
} /* util namespace */
