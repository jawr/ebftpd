//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
#include "ftp/online.hpp"
#include "fs/mode.hpp"

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
    ("config-path,c", po::value<std::string>(), "specify location of config file")
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

bool AlreadyRunning()
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
      return true;
    }
  }
  return false;
}

bool Daemonise(bool foreground)
{
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
  
  const std::string& pidfile = cfg::Get().Pidfile();
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

int Main(int argc, char** argv)
{
  logs::SetThreadIDPrefix('P' /* parent */);
  
  std::string configPath;
  bool foreground; 

  if (!ParseOptions(argc, argv, foreground, configPath)) return 1;

  logs::InitialisePreConfig();
  
  logs::Debug("Starting %1%..", programFullname);
  auto byeExit = util::MakeScopeExit([]() { logs::Debug("Bye!"); });
  
  cmd::rfc::Factory::Initialise();
  cmd::site::Factory::Initialise();
  cfg::Config::PopulateACLKeywords(cmd::site::Factory::ACLKeywords());
  ftp::InitialisePortAllocators();
  ftp::InitialiseAddrAllocators();
  fs::InitialiseUmask();
  
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
  
  {
    util::Error e = signals::Initialise(util::path::Join(cfg::Get().Datapath(), "logs"));
    if (!e)
    {
      logs::Error("Failed to setup signal handlers: %1%", e.Message());
      return 1;
    }
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
      const cfg::Config& config = cfg::Get();
      logs::Debug("Initialising TLS context..");
      util::net::TLSServerContext::Initialise(programName, config.TlsCertificate(), config.TlsCiphers());
      util::net::TLSClientContext::Initialise(config.TlsCertificate(), config.TlsCiphers());
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

  if (!AlreadyRunning())
  {
    if (!ftp::Server::Initialise(cfg::Get().ValidIp(), cfg::Get().Port()))
    {
      logs::Error("Listener failed to initialise!");
      return 1;
    }
    
    if (Daemonise(foreground))
    {
      try
      {
        ftp::OnlineWriter::Initialise(ftp::SharedMemoryID(), cfg::Config::MaxOnline().Total());
        signals::Handler::StartThread();
        db::Replicator::Get().Start();
        ftp::Server::Get().StartThread();
        ftp::Server::Get().JoinThread();
        db::Replicator::Get().Stop();
        ftp::Server::Cleanup();
        signals::Handler::StopThread();
        ftp::OnlineWriter::Cleanup();
      }
      catch (const util::SystemError& e)
      {
        logs::Error("Shared memory segment failed to initialise: %1%", e.Message());
        return 1;
      }
    }
  }
  
  return 0;
}

int main(int argc, char** argv)
{
  _exit(Main(argc, argv)); // horrible work around for mongodb driver bug
}
