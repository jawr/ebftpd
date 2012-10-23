#include <memory>
#include "ftp/listener.hpp"
#include "util/net/tlscontext.hpp"
#include "util/net/error.hpp"
#include "logs/logs.hpp"
#include "fs/owner.hpp"
#include "fs/path.hpp"
#include "cfg/config.hpp"
#include "cfg/get.hpp"
#include "cfg/exception.hpp"
#include "ftp/portallocator.hpp"
#include "ftp/addrallocator.hpp"
#include "db/interface.hpp"
#include "db/exception.hpp"
#include "acl/usercache.hpp"
#include "acl/groupcache.hpp"

#include "version.hpp"

extern const std::string programName = "ebftpd";
extern const std::string programFullname = programName + " " + std::string(version);

#ifndef TEST
int main(int argc, char** argv)
{
  std::cout << "Starting " << programFullname << " .. " << std::endl;

  std::shared_ptr<cfg::Config> config;
  try
  {
    config.reset(new cfg::Config("ftpd.conf"));
    cfg::UpdateShared(config);
  }
  catch (const cfg::ConfigError& e)
  {
    logs::error << "Failed to load config: " << e.Message() << logs::endl;
    return 1;
  }

  try
  {
    const std::string& certificate = cfg::Get().TlsCertificate().ToString();
    util::net::TLSServerContext::Initialise(certificate, "");
  }
  catch (const util::net::NetworkError& e)
  {
    logs::error << "TLS failed to initialise: " << e.Message() << logs::endl;
    return 1;
  }
  
  ftp::AddrAllocator<ftp::AddrType::Active>::SetAddrs(config->ActiveAddr());
  ftp::AddrAllocator<ftp::AddrType::Passive>::SetAddrs(config->PasvAddr());
  ftp::PortAllocator<ftp::PortType::Active>::SetPorts(config->ActivePorts());
  ftp::PortAllocator<ftp::PortType::Passive>::SetPorts(config->PasvPorts());
  
  try
  {
    db::Initalize();
  }
  catch (const db::DBError& e)
  {
    logs::error << "DB failed to initialise: " << e.Message() << logs::endl;
    return 1;
  }
  
  fs::OwnerCache::Start();
  
  int exitStatus = 0;
  
  if (!ftp::Listener::Initialise(cfg::Get().ValidIp(), cfg::Get().Port()))
  {
    logs::error << "Listener failed to initialise!" << logs::endl;
    exitStatus = 1;
  }
  else
  {  
    ftp::Listener::StartThread();
    ftp::Listener::JoinThread();
  }
  
  fs::OwnerCache::Stop();
  
  (void) argc;
  (void) argv;
  
  return exitStatus;
}

#endif
