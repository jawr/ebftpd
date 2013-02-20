#include <memory>
#include <cstring>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <unistd.h>
#include "ftp/server.hpp"
#include "util/net/tlscontext.hpp"
#include "util/net/error.hpp"
#include "logs/logs.hpp"
#include "fs/owner.hpp"
#include "cfg/config.hpp"
#include "cfg/get.hpp"
#include "cfg/error.hpp"
#include "ftp/portallocator.hpp"
#include "ftp/addrallocator.hpp"
#include "acl/util.hpp"
#include "ftp/client.hpp"
#include "util/error.hpp"
#include "util/daemonise.hpp"
#include "cmd/rfc/factory.hpp"
#include "cmd/site/factory.hpp"
#include "signals/signal.hpp"
#include "text/factory.hpp"
#include "text/error.hpp"
#include "util/path/path.hpp"
#include "db/initialise.hpp"
#include "util/scopeguard.hpp"

#include "version.hpp"

extern const std::string programName = "ebftpd";
extern const std::string programFullname = programName + " " + std::string(version);

void DisplayHelp(char* argv0, boost::program_options::options_description& desc)
{
  std::cout << "usage: " << argv0 << " [options]" << std::endl;
  std::cout << desc;
}

void DisplayVersion()
{
  std::cout << programFullname << std::endl;
}

bool ParseOptions(int argc, char** argv, bool& foreground, std::string& configPath)
{
  namespace po = boost::program_options;
  po::options_description desc("supported options");
  desc.add_options()
    ("help,h", "display this help message")
    ("version,v", "display version")
    ("config-path,c", po::value<std::string>(),
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
  
  if (vm.count("config-path")) configPath = vm["config-path"].as<std::string>();
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

int main(int argc, char** argv)
{
  std::string configPath;
  bool foreground; 
  
  if (!ParseOptions(argc, argv, foreground, configPath)) return 1;
  
  logs::debug << "Starting " << programFullname << " .. " << logs::endl;
  auto byeExit = util::MakeScopeExit([]() { logs::debug << "Bye!" << logs::endl; });
  
  {
    util::Error e = signals::Initialise();
    if (!e)
    {
      logs::error << "Failed to setup signal handlers: " << e.Message() << logs::endl;
      return 1;
    }
    signals::Handler::StartThread();
  }
  
  auto signalsExit = util::MakeScopeExit([]() { signals::Handler::StopThread(); });
  
  cmd::rfc::Factory::Initialise();
  cmd::site::Factory::Initialise();
  cfg::Config::PopulateACLKeywords(cmd::site::Factory::ACLKeywords());
  ftp::InitialisePortAllocators();
  ftp::InitialiseAddrAllocators();
  
  try
  {
    logs::debug << "Loading config file.." << logs::endl;
    cfg::UpdateShared(cfg::Config::Load(configPath));
  }
  catch (const cfg::ConfigError& e)
  {
    logs::error << "Failed to load config: " << e.Message() << logs::endl;
    return 1;
  }

  logs::Initialise(util::path::Join(cfg::Get().Datapath(), "logs"));
  
  if (cfg::Get().TlsCertificate().empty())
  {
    logs::debug << "No TLS certificate set in config, TLS disabled." << logs::endl;
  }
  else
  {
    try
    {
      logs::debug << "Initialising TLS context.." << logs::endl;
      util::net::TLSServerContext::Initialise(
          cfg::Get().TlsCertificate(), cfg::Get().TlsCiphers());
      util::net::TLSClientContext::Initialise(
          cfg::Get().TlsCertificate(), cfg::Get().TlsCiphers());
    }
    catch (const util::net::NetworkError& e)
    {
      logs::error << "TLS failed to initialise: " << e.Message() << logs::endl;
      return 1;
    }
  }

  logs::debug << "Initialising Templates.." << logs::endl;
  try
  {
    text::Factory::Initialize();
  }
  catch (const text::TemplateError& e)
  {
    logs::error << "Templates failed to initialise: " << e.Message() << logs::endl;
    return 1;
  }
  
  if (!db::Initialise()) return 1;
  auto dbExit = util::MakeScopeExit([]() { db::Cleanup(); });
  
  if (!acl::CreateDefaults())
  {
    logs::error << "Error while creating default user and group" << logs::endl;
    return 1;
  }
  
  int exitStatus = 0;
  if (!ftp::Server::Initialise(cfg::Get().ValidIp(), cfg::Get().Port()))
  {
    logs::error << "Listener failed to initialise!" << logs::endl;
    exitStatus = 1;
  }
  else if (Daemonise(foreground))
  {
    ftp::Server::StartThread();
    ftp::Server::JoinThread();
  }
  
  return exitStatus;
}
