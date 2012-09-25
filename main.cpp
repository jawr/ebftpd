#include "ftp/listener.hpp"
#include "util/net/tlscontext.hpp"
#include "util/net/error.hpp"
#include "logger/logger.hpp"

#ifndef TEST

int main(int argc, char** argv)
{
  char *certificate = getenv("FTPD_CERT");
  if (!certificate)
  {
    logger::error << "set environment variable FTPD_CERT to your certificate file" << logger::endl;
    return 1;
  }
  
  try
  {
    util::net::TLSServerContext::Initialise(certificate);
  }
  catch (const util::net::NetworkError& e)
  {
    logger::error << "TLS failed to initialise: " << e.Message() << logger::endl;
    return 1;
  }

  ftp::Listener listener("127.0.0.1", 1234);  
  if (!listener.Initialise())
  {
    logger::error << "Listener failed to initialise!" << logger::endl;
    return 1;
  }
  
  listener.Start();
  listener.Join();
  
  (void) argc;
  (void) argv;
}

#endif
