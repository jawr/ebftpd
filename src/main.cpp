#include <memory>
#include <cstring>
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
#include "util/error.hpp"
#include "util/daemonise.hpp"
#include "cmd/site/factory.hpp"
#include "signals/signal.hpp"

#include "version.hpp"

extern const std::string programName = "ebftpd";
extern const std::string programFullname = programName + " " + std::string(version);

namespace
{
std::string configFile = "ftpd.conf";
}

void LoadConfig()
{
  logs::debug << "Loading config file.." << logs::endl;
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

void DisplayVersion()
{
  std::cout << programFullname << std::endl;
}

bool ParseOptions(int argc, char** argv, bool& foreground)
{
  namespace po = boost::program_options;
  po::options_description desc("supported options");
  desc.add_options()
    ("help,h", "display this help message")
    ("version,v", "display version")
    ("config-file,c", po::value<std::string>(),
     "specify location of config file")
    ("foreground,f", "run server in foreground")
    ("siteop-only,s", "run server in siteop only mode");

  po::variables_map vm;
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
  
  if (vm.count("version"))
  {
    DisplayVersion();
    return false;
  }
  
  if (vm.count("config-file")) configFile = vm["config-file"].as<std::string>();
  if (vm.count("siteop-only")) ftp::Client::SetSiteopOnly();
  foreground = vm.count("foreground") > 0;
  
  return true;
}

bool Daemonise(bool foreground)
{
  const std::string& pidfile = cfg::Get().Pidfile();
  if (!pidfile.empty())
  {
    util::Error e = util::daemonise::NotRunning(pidfile);
    if (!e)
    {
      if (e.ValidErrno())
        logs::error << "Failed to read the pidfile: " << e.Message() << logs::endl;
      else
        logs::error << "Server already running. If it's not, delete the pid file at: " << pidfile << logs::endl;
      return false;
    }
  }
  
  if (!foreground)
  {
    logs::debug << "Forking into the background.." << logs::endl;
    util::Error e = util::daemonise::Daemonise();
    if (!e)
    {
      logs::error << "Failed to daemonise server process: " 
                  << e.Message() << logs::endl;
      return false;
    }
    else
      logs::NoStdout();
  }
  
  if (!pidfile.empty())
  {
    util::Error e = util::daemonise::CreatePIDFile(pidfile);
    if (!e)
    {
      logs::error << "Failed to create pid file: " << e.Message() << logs::endl;
    }
  }
  
  return true;
}

#ifndef TEST
int main(int argc, char** argv)
{
  bool foreground; 
  if (!ParseOptions(argc, argv, foreground)) return 1;

  logs::debug << "Starting " << programFullname << " .. " << logs::endl;
  cfg::Config::PopulateACLKeywords(cmd::site::Factory::ACLKeywords());
  
  try
  {
    LoadConfig();
  }
  catch (const cfg::ConfigError& e)
  {
    logs::error << "Failed to load config: " << e.Message() << logs::endl;
    return 1;
  }

  logs::Initialise(cfg::Get().Datapath() + "/logs");
  if (!Daemonise(foreground)) return 1;
  
  {
    util::Error e = signals::Initialise();
    if (!e)
    {
      logs::error << "Failed to setup signal handlers: " << e.Message() << logs::endl;
      return 1;
    }
  }
  
  try
  {
    logs::debug << "Initialising TLS context.." << logs::endl;
    util::net::TLSServerContext::Initialise(
        cfg::Get().TlsCertificate().ToString(), 
        cfg::Get().TlsCiphers().ToString());
    util::net::TLSClientContext::Initialise(
        cfg::Get().TlsCertificate().ToString(), 
        cfg::Get().TlsCiphers().ToString());
  }
  catch (const util::net::NetworkError& e)
  {
    logs::error << "TLS failed to initialise: " << e.Message() << logs::endl;
    return 1;
  }
    
  try
  {
    db::Initialize();
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
    signals::Handler::StartThread();
    ftp::Listener::StartThread();
    ftp::Listener::JoinThread();
    signals::Handler::StopThread();
  }

  db::Cleanup();
  fs::OwnerCache::Stop();

  logs::debug << "Bye!" << logs::endl;
  
  return exitStatus;
}

#endif
