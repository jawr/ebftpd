#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include "util/net/tlscontext.hpp"
#include "util/net/tlserror.hpp"

namespace util { namespace net
{

std::auto_ptr<TLSClientContext> TLSContext::client;
std::auto_ptr<TLSServerContext> TLSContext::server;

TLSContext::~TLSContext()
{
  if (context) SSL_CTX_free(context);
}

TLSContext::TLSContext(const std::string& certificate, const std::string& ciphers) :
  context(0),
  certificate(certificate),
  ciphers(ciphers)
{
}

void TLSContext::Initialise()
{
  InitialiseOpenSSL();
  CreateContext();
  LoadCertificate();
  InitialiseSessionCaching();
  SelectCiphers();
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
        SSL_CTX_use_PrivateKey_file(context, certificate.c_str(), SSL_FILETYPE_PEM) != 1 ||
        SSL_CTX_check_private_key(context) != 1) throw TLSProtocolError();
  }
}

void TLSContext::SelectCiphers()
{
 if (!ciphers.empty() && SSL_CTX_set_cipher_list(context, ciphers.c_str()) != 1)
    throw TLSError("No valid ciphers selected");
}

TLSClientContext::TLSClientContext(const std::string& certificate,
                                   const std::string& ciphers) :
  TLSContext(certificate, ciphers)
{
}

void TLSClientContext::CreateContext()
{
  context = SSL_CTX_new(SSLv23_client_method());
  if (!context) throw TLSProtocolError();
  SSL_CTX_set_options(context,/* SSL_OP_NO_SSLv2 | */SSL_OP_ALL);
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
  context = SSL_CTX_new(SSLv23_server_method());
  if (!context) throw TLSProtocolError();
  SSL_CTX_set_options(context, SSL_OP_NO_SSLv2 | SSL_OP_ALL);
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
