#include <memory>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
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
#include "ftp/client.hpp"

#include "version.hpp"

extern const std::string programName = "ebftpd";
extern const std::string programFullname = programName + " " + std::string(version);

namespace
{
std::string configFile = "ftpd.conf";
}

void LoadConfig()
{
  cfg::UpdateShared(std::shared_ptr<cfg::Config>(new cfg::Config(configFile)));
  
  ftp::AddrAllocator<ftp::AddrType::Active>::SetAddrs(cfg::Get().ActiveAddr());
  ftp::AddrAllocator<ftp::AddrType::Passive>::SetAddrs(cfg::Get().PasvAddr());
  ftp::PortAllocator<ftp::PortType::Active>::SetPorts(cfg::Get().ActivePorts());
  ftp::PortAllocator<ftp::PortType::Passive>::SetPorts(cfg::Get().PasvPorts());
}

void DisplayHelp(char* argv0, boost::program_options::options_description& desc)
{
  std::cout << "usage: " << argv0 << " [options]" << std::endl;
  std::cout << desc;
}

bool ParseOptions(int argc, char** argv, boost::program_options::variables_map& vm)
{
  namespace po = boost::program_options;
  po::options_description desc("supported options");
  desc.add_options()
    ("help,h", "display this help message")
    ("config-file,c", po::value<std::string>(),
     "specify location of config file")
    ("foreground,f", "run server in foreground")
    ("siteop-only,s", "run server in siteop only mode");

  try
  {
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm); 
  }
  catch (const boost::program_options::error& e)
  {
    std::cerr << e.what() << std::endl;
    DisplayHelp(argv[0], desc);
    return false;
  }
  
  if (vm.count("help"))
  {
    DisplayHelp(argv[0], desc);
    return false;
  }
  
  if (vm.count("config-file")) configFile = vm["config-file"].as<std::string>();
  if (vm.count("siteop-only")) ftp::Client::SetSiteopOnly();
  
  return true;
}

#ifndef TEST
int main(int argc, char** argv)
{
  boost::program_options::variables_map vm;
  
  if (!ParseOptions(argc, argv, vm)) return 1;

  std::cout << "Starting " << programFullname << " .. " << std::endl;

  try
  {
    LoadConfig();
  }
  catch (const cfg::ConfigError& e)
  {
    logs::error << "Failed to load config: " << e.Message() << logs::endl;
    return 1;
  }

  try
  {
    util::net::TLSServerContext::Initialise(
        cfg::Get().TlsCertificate().ToString(), "");
  }
  catch (const util::net::NetworkError& e)
  {
    logs::error << "TLS failed to initialise: " << e.Message() << logs::endl;
    return 1;
  }
    
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

  db::Cleanup();
  fs::OwnerCache::Stop();
  
  return exitStatus;
}

#endif
