#include <memory>
#include "ftp/listener.hpp"
#include "util/net/tlscontext.hpp"
#include "util/net/error.hpp"
#include "logger/logger.hpp"
#include "fs/owner.hpp"
#include "fs/path.hpp"
#include "cfg/config.hpp"
#include "cfg/get.hpp"
#include "cfg/exception.hpp"
#include "ftp/portallocator.hpp"
#include "ftp/addrallocator.hpp"

#ifndef TEST

int main(int argc, char** argv)
{
  std::shared_ptr<cfg::Config> config;
  try
  {
    config.reset(new cfg::Config("ftpd.conf"));
    cfg::UpdateShared(config);
  }
  catch (const cfg::ConfigError& e)
  {
    logger::error << e.what() << logger::endl;
    return 1;
  }
  
  try
  {
    const std::string& certificate = cfg::Get().TlsCertificate().ToString();
    util::net::TLSServerContext::Initialise(certificate, "");
  }
  catch (const util::net::NetworkError& e)
  {
    logger::error << "TLS failed to initialise: " << e.Message() << logger::endl;
    return 1;
  }
  
  ftp::AddrAllocator<ftp::AddrType::Active>::SetAddrs(config->ActiveAddr());
  ftp::AddrAllocator<ftp::AddrType::Passive>::SetAddrs(config->PasvAddr());
  ftp::PortAllocator<ftp::PortType::Active>::SetPorts(config->ActivePorts());
  ftp::PortAllocator<ftp::PortType::Passive>::SetPorts(config->PasvPorts());
  fs::OwnerCache::Start();
  
  
  ftp::Listener listener(cfg::Get().ListenAddr(), cfg::Get().Port());  
  if (!listener.Initialise())
  {
    logger::error << "Listener failed to initialise!" << logger::endl;
    return 1;
  }
  
  listener.Start();
  listener.Join();
  
  fs::OwnerCache::Stop();
  
  (void) argc;
  (void) argv;
}

#endif
