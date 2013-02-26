#include <memory>
#include <cstring>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <unistd.h>
#include "main.hpp"
#include "ftp/server.hpp"
#include "ftp/task/task.hpp"
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
#include "db/replicator.hpp"

#include "version.hpp"

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
        logs::Error("Failed to read the pidfile: %1%", e.Message());
      else
        logs::Error("Server already running. If it's not, delete the pid file at: %1%", pidfile);
      return false;
    }
  }
  
  if (!foreground)
  {
    logs::Debug("Forking into the background..");
    util::Error e = util::daemonise::Daemonise();
    if (!e)
    {
      logs::Error("Failed to daemonise server process: %1%", e.Message());
      return false;
    }
  }
  
  if (!pidfile.empty())
  {
    util::Error e = util::daemonise::CreatePIDFile(pidfile);
    if (!e)
    {
      logs::Error("Failed to create pid file: %1%", e.Message());
    }
  }
  
  return true;
}

int main(int argc, char** argv)
{
  std::string configPath;
  bool foreground; 
  
  if (!ParseOptions(argc, argv, foreground, configPath)) return 1;

  logs::InitialisePreConfig();
  
  logs::Debug("Starting %1%..", programFullname);
  auto byeExit = util::MakeScopeExit([]() { logs::Debug("Bye!"); });
  
  {
    util::Error e = signals::Initialise();
    if (!e)
    {
      logs::Error("Failed to setup signal handlers: %1%", e.Message());
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
    logs::Debug("Loading config file..");
    cfg::UpdateShared(cfg::Config::Load(configPath));
  }
  catch (const cfg::ConfigError& e)
  {
    logs::Error("Failed to load config: %1%", e.Message());
    return 1;
  }
  
  if (!logs::InitialisePostConfig()) return 1;
  
  if (cfg::Get().TlsCertificate().empty())
  {
    logs::Debug("No TLS certificate set in config, TLS disabled.");
  }
  else
  {
    try
    {
      logs::Debug("Initialising TLS context..");
      util::net::TLSServerContext::Initialise(
          cfg::Get().TlsCertificate(), cfg::Get().TlsCiphers());
      util::net::TLSClientContext::Initialise(
          cfg::Get().TlsCertificate(), cfg::Get().TlsCiphers());
    }
    catch (const util::net::NetworkError& e)
    {
      logs::Error("TLS failed to initialise: %1%", e.Message());
      return 1;
    }
  }

  logs::Debug("Initialising Templates..");
  try
  {
    text::Factory::Initialize();
  }
  catch (const text::TemplateError& e)
  {
    logs::Error("Templates failed to initialise: %1%", e.Message());
    return 1;
  }
  
  if (!db::Initialise([](acl::UserID uid)
        { std::make_shared<ftp::task::UserUpdate>(uid)->Push(); }))
  {
    return 1;
  }
  
  if (!acl::CreateDefaults())
  {
    logs::Error("Error while creating root user and group and default user template");
    return 1;
  }
  
  int exitStatus = 0;
  if (!ftp::Server::Initialise(cfg::Get().ValidIp(), cfg::Get().Port()))
  {
    logs::Error("Listener failed to initialise!");
    exitStatus = 1;
  }
  else if (Daemonise(foreground))
  {
    db::Replicator::Get().Start();
    ftp::Server::StartThread();
    ftp::Server::JoinThread();
    db::Replicator::Get().Stop();
  }
  
  return exitStatus;
}
