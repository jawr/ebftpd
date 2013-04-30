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

#include <pthread.h>
#include <csignal>
#include <cstring>
#include <iomanip>
#include <functional>
#include "ftp/clientimpl.hpp"
#include "logs/logs.hpp"
#include "util/verify.hpp"
#include "util/error.hpp"
#include "util/scopeguard.hpp"
#include "util/net/tcplistener.hpp"
#include "cmd/rfc/factory.hpp"
#include "acl/path.hpp"
#include "acl/types.hpp"
#include "cfg/get.hpp"
#include "util/misc.hpp"
#include "main.hpp"
#include "util/net/identclient.hpp"
#include "util/string.hpp"
#include "ftp/counter.hpp"
#include "acl/flags.hpp"
#include "db/mail/mail.hpp"
#include "db/stats/protocol.hpp"
#include "ftp/error.hpp"
#include "cmd/error.hpp"
#include "exec/cscript.hpp"
#include "util/net/resolver.hpp"
#include "acl/misc.hpp"
#include "util/misc.hpp"
#include "ftp/task/task.hpp"
#include "ftp/online.hpp"
#include "fs/directory.hpp"

namespace ftp
{

std::atomic_bool ClientImpl::siteopOnly(false);

ClientImpl::ClientImpl(Client& parent) :
  parent(parent),
  data(parent), 
  loginGuard(parent),
  userUpdated(false),
  state(ClientState::LoggedOut),
  passwordAttemps(0),
  xdupeMode(xdupe::Mode::Disabled),
  kickLogin(false),
  idleTimeout(boost::posix_time::seconds(cfg::Get().IdleTimeout().Timeout())),
  ident("*")
{
}

ClientImpl::~ClientImpl()
{
}

void ClientImpl::SetState(ClientState state)
{
  assert(state != ClientState::LoggedIn); // these 2 states have own setter
  assert(state != ClientState::WaitingPassword);
  
  bool logout;
  
  {
    std::lock_guard<std::mutex> lock(mutex);
    logout = state == ClientState::Finished && this->state == ClientState::LoggedIn;
    this->state = state;
  }

  if (logout)
  {
    loginGuard.Logout();
    logs::Event("LOGOUT", logs::QuoteOff(), 
                "ident_address", Ident(LogAddresses::Normal) + "@" + Hostname(LogAddresses::Normal), 
                "ip", logs::Brackets('(', ')'), IP(LogAddresses::Normal), 
                 logs::QuoteOn(), "user", user->Name(), 
                "group", user->PrimaryGroup(), 
                "tagline", user->Tagline());
  }
}

void ClientImpl::SetLoggedIn(bool kicked)
{
  auto result = loginGuard.Login(kicked, boost::this_thread::get_id());
  switch (result)
  {
    case CounterResult::PersonalFail  :
    {
      std::ostringstream os;
      os << "You've reached your maximum of " << user->NumLogins() << " login(s).";
      throw util::RuntimeError(os.str());
    }
    case CounterResult::GlobalFail    :
    {
      throw util::RuntimeError("The server has reached it's maximum number of logged in users.");
    }
    case CounterResult::Okay          :
      break;
  }

  if (user->IdleTime() == -1)
    SetIdleTimeout(cfg::Get().IdleTimeout().Timeout());
  else
    SetIdleTimeout(boost::posix_time::seconds(user->IdleTime()));

  {
    std::lock_guard<std::mutex> lock(mutex);
    state = ClientState::LoggedIn;
    loggedInAt = boost::posix_time::second_clock::local_time();
  }

  logs::Event("LOGIN", logs::QuoteOff(), 
              "ident address", Ident(LogAddresses::Normal) + '@' + Hostname(LogAddresses::Normal), 
              "ip", logs::Brackets('(', ')'), IP(LogAddresses::Normal), 
              logs::QuoteOn(), "user", user->Name(), 
              "group", user->PrimaryGroup(), 
              "tagline", user->Tagline());
}

void ClientImpl::SetWaitingPassword(const acl::User& user, bool kickLogin)
{
  {
    std::lock_guard<std::mutex> lock(mutex);
    state = ClientState::WaitingPassword;
    this->user = std::move(user);
    this->kickLogin = kickLogin;
  }
}

bool ClientImpl::CheckState(ClientState reqdState)
{
  if (state == reqdState || reqdState == ClientState::AnyState) return true;
  if (state == ClientState::LoggedIn)
    control.Reply(ftp::NotLoggedIn, "Already logged in.");
  else if (state == ClientState::WaitingPassword)
    control.Reply(ftp::BadCommandSequence, "Expecting PASS comamnd.");
  else if (state == ClientState::LoggedOut &&
           reqdState == ClientState::WaitingPassword)
    control.Reply(ftp::BadCommandSequence, "Expecting USER command first.");
  else if (reqdState == ClientState::NotBeforeAuth)
  {
    if (!control.IsTLS())
      control.Reply(ftp::BadCommandSequence, "AUTH command must be issued first.");
    else
      return true;
  }
  else if (state == ClientState::LoggedOut)
      control.Reply(ftp::NotLoggedIn, "Not logged in.");
  assert(state != ClientState::Finished);
  return false;
}

bool ClientImpl::VerifyPassword(const std::string& password)
{
  ++passwordAttemps;
  return user->VerifyPassword(password);
}

bool ClientImpl::PasswordAttemptsExceeded() const
{
  return passwordAttemps >= maxPasswordAttemps;
}

bool ClientImpl::Accept(util::net::TCPListener& server)
{
  try
  {
    control.Accept(server);
    ip = control.RemoteEndpoint().IP().IsMappedv4() ?
         control.RemoteEndpoint().IP().ToUnmappedv4().ToString() :
         control.RemoteEndpoint().IP().ToString();
    return true;
  }
  catch(const util::net::NetworkError& e)
  {
    SetState(ClientState::Finished);
    logs::Error("Error while accepting new client: %1%", e.Message());
    return false;
  }
}

void ClientImpl::DisplayBanner()
{
  const cfg::Config& config = cfg::Get();
  if (!config.Banner().empty())
  {
    std::string banner;
    if (util::ReadFileToString(config.Banner(), banner))
    {
      control.Reply(ftp::ServiceReady, banner);
      return;
    }
  }

  control.Format(ftp::ServiceReady, config.LoginPrompt());
}

void ClientImpl::IdleReset(std::string commandLine)
{
  for (auto & mask : cfg::Get().IdleCommands())
    if (util::WildcardMatch(mask, commandLine, true))
      return;
  idleTime = boost::posix_time::second_clock::local_time();
  idleExpires = idleTime + idleTimeout;
}

void ClientImpl::ExecuteCommand(const std::string& commandLine)
{
  if (commandLine.empty()) return;
  std::vector<std::string> args;
  util::Split(args, commandLine, " ", true);
  if (args.empty()) throw ProtocolError("Empty command.");
  
  std::string argStr(commandLine.substr(args[0].length()));
  util::Trim(argStr);
  util::ToUpper(args[0]);
  
  currentCommand = args[0];
  if (!argStr.empty()) currentCommand += " " + argStr;
  
  if (State() == ClientState::LoggedIn)
  {
    OnlineWriter::Get().Command(boost::this_thread::get_id(), currentCommand);
  }
  
  cmd::rfc::CommandDefOptRef def(cmd::rfc::Factory::Lookup(args[0]));
  if (!def)
  {
    control.Reply(ftp::CommandUnrecognised, "Command not understood");
  }
  else if (!def->CheckArgs(args))
  {
    control.Reply(ftp::SyntaxError, "Syntax: " + def->Syntax());
  }
  else if (CheckState(def->RequiredState()) &&
           (state != ClientState::LoggedIn ||
            exec::Cscripts(parent, args[0], currentCommand, exec::CscriptType::Pre, 
                def->FailCode())))
  {
    cmd::CommandPtr command(def->Create(parent, argStr, args));
    if (!command)
    {
      control.Reply(ftp::NotImplemented, "Command not implemented");
    }
    else
    {
      try
      {
        command->Execute();

        if (state == ClientState::LoggedIn)
          exec::Cscripts(parent, args[0], currentCommand, exec::CscriptType::Post, 
                  ftp::ActionNotOkay);
      }
      catch (const cmd::SyntaxError&)
      {
        control.Reply(ftp::SyntaxError, "Syntax: " + def->Syntax());
      }
      catch (const cmd::NoPostScriptError&)
      {
        // do nothing - skip post cscript
      }

      IdleReset(commandLine);
    }
  }
  
  currentCommand = "";
  
  if (State() == ClientState::LoggedIn)
  {
    OnlineWriter::Get().Idle(boost::this_thread::get_id());
  }
}

bool ClientImpl::ReloadUser()
{
  userUpdated = false;

  auto optUser = acl::User::Load(user->ID());
  if (!optUser)
  {
    logs::Error("Failed to reload user from cache for: %1%", user->Name());
    
    SetState(ClientState::Finished);
    return false; 
  }
  
  if (optUser->HasFlag(acl::Flag::Deleted))
  {
    SetState(ClientState::Finished);
    return false;
  }

  
  logs::Debug("Reloaded user profile");
  
  std::lock_guard<std::mutex> lock(mutex);
  user = std::move(*optUser);
  
  return true;
}

void ClientImpl::Handle()
{
  namespace pt = boost::posix_time;

  pt::time_duration timeout;
  pt::time_duration* timeoutPtr = nullptr;
  
  while (State() != ClientState::Finished)
  {
    if (State() != ClientState::LoggedIn || user->IdleTime() == 0) 
      timeoutPtr = nullptr;
    else
    {     
      timeout = idleExpires - pt::second_clock::local_time();
      timeoutPtr = &timeout;
    }
    
    std::string command = control.NextCommand(timeoutPtr);    
    if (userUpdated && !ReloadUser()) break;
    ExecuteCommand(command);
    cfg::UpdateLocal();
  }
}

void ClientImpl::Interrupt()
{
  SetState(ClientState::Finished);
  Stop();
  control.Interrupt();
  data.Interrupt();
  child.Interrupt();
}

void ClientImpl::LookupIdent()
{
  if (!cfg::Get().IdentLookup() || ident != "*") return;
  
  try
  {
    util::net::IdentClient identClient(control.LocalEndpoint(), control.RemoteEndpoint());
    ident = identClient.Ident();
  }
  catch (util::net::NetworkError& e)
  {
    logs::Error("Unable to lookup ident for connection from %1%: %2%",
                control.RemoteEndpoint(), e.Message());
  }
}

bool ClientImpl::ConfirmCommand(const std::string& argStr)
{
  std::string command = util::CompressWhitespaceCopy(argStr);
  if (command != confirmCommand)
  {
    confirmCommand = command;
    return false;
  }
  confirmCommand.clear();
  return true;
}

void ClientImpl::LogTraffic() const
{
  db::stats::ProtocolUpdate(user ? user->ID() : -1,
                            (control.BytesWrite() + data.BytesWrite()) / 1024,
                            (control.BytesRead() + data.BytesRead()) / 1024);
}

bool ClientImpl::PostCheckAddress()
{
  return acl::IdentIPAllowed(user->ID(), ident + "@" + IP()) ||
        (IP() != Hostname() && acl::IdentIPAllowed(user->ID(), ident + "@" + Hostname()));
}

bool ClientImpl::PreCheckAddress()
{
  if (!acl::IPAllowed(IP()) && (IP() != Hostname() && !acl::IPAllowed(Hostname())))
  {
    logs::Security("BADADDRESS", "Refused connection from unknown address: %1%", HostnameAndIP(LogAddresses::Error));
    return false;
  }
  
  return true;
}

void ClientImpl::HostnameLookup()
{
  if (!cfg::Get().DNSLookup() || !hostname.empty()) return;
  
  try
  { 
  
    std::string hostname = util::net::ReverseResolve(util::net::IPAddress(ip));
    
    {
      std::lock_guard<std::mutex> lock(mutex);
      this->hostname = hostname;
    }
  }
  catch (const util::net::NetworkError&)
  {
    {
      std::lock_guard<std::mutex> lock(mutex);
      this->hostname = ip;
    }
  }
}

std::string ClientImpl::SanitiseAddress(std::string address, LogAddresses log) const
{
  const cfg::Config& config = cfg::Get();
  switch (log)
  {
    case LogAddresses::Error      :
    {
      if (config.LogAddresses() == cfg::LogAddresses::Never)
        address = "disabled";
      break;
    }
    case LogAddresses::Normal     :
    {
      if (config.LogAddresses() != cfg::LogAddresses::Always)
        address = "disabled";
      break;
    }
    case LogAddresses::NotLogging :
    {
      break;
    }
  }
  return address;
}

std::string ClientImpl::IP(LogAddresses log) const
{
  std::lock_guard<std::mutex> lock(mutex);
  return SanitiseAddress(ip, log);
}

std::string ClientImpl::Ident(LogAddresses log) const
{
  std::lock_guard<std::mutex> lock(mutex);
  return SanitiseAddress(ident, log);
}

std::string ClientImpl::Hostname(LogAddresses log) const
{
  std::lock_guard<std::mutex> lock(mutex);
  return SanitiseAddress(hostname, log);
}

std::string ClientImpl::HostnameAndIP(LogAddresses log) const
{
  std::ostringstream os;
  std::string hostname = Hostname(log);
  std::string ip = IP(log);
  if (hostname.empty()) os << ip;
  else os << hostname;
  os << "(" << ip << ")";
  return os.str();
}

bool ClientImpl::IdntUpdate(const std::string& ident, std::string ip,
                        const std::string& hostname)
{
  try
  {
    util::net::IPAddress ipa(ip);
    if (ipa.IsMappedv4()) ip = ipa.ToUnmappedv4().ToString();
  }
  catch (const util::net::InvalidIPAddressError&)
  {
    return false;
  }
  
  {
    std::lock_guard<std::mutex> lock(mutex);
    this->ident = ident;
    this->ip = ip;
    if (ip != hostname) this->hostname = hostname;
  }
  
  return true;
}

bool ClientImpl::IdntParse(const std::string& command)
{
  std::vector<std::string> args;
  util::Split(args, command, " ");
  if (args.size() != 2) return false;
  
  auto pos1 = args[1].find_first_of('@');
  if (pos1 == std::string::npos) return false;
  
  auto pos2 = args[1].find_last_of(':');
  if (pos2 == std::string::npos || pos2 <= pos1) return false;
  
  std::string ident(args[1], 0, pos1);
  std::string ip(args[1], pos1 + 1, pos2 - pos1 - 1);
  std::string hostname(args[1], pos2 + 1);
  
  if (ident.empty() || hostname.empty() || ip.empty()) return false;

  return IdntUpdate(ident, ip, hostname);
}

void ClientImpl::InnerRun()
{
  if (!cfg::Get().IsBouncer(ip))
  {
    if (cfg::Get().BouncerOnly() && !control.RemoteEndpoint().IP().IsLoopback())
    {
      logs::Security("NONBOUNCER", "Refused connection not from a bouncer address: %1%", HostnameAndIP(LogAddresses::Error));
      return;
    }
  }
  else
  {
    std::string command = control.WaitForIdnt();
    if (command.empty())
    {
      if (cfg::Get().BouncerOnly())
      {
        logs::Security("IDNTTIMEOUT", "Timeout while waiting for IDNT command from bouncer: ", HostnameAndIP(LogAddresses::Error));
        return;
      }
    }
    else
    if (!IdntParse(command))
    {
      logs::Security("BADIDNT", "Malformed IDNT command from bouncer: ", HostnameAndIP(LogAddresses::Error));
      return;
    }
  }

  HostnameLookup();

  if (!PreCheckAddress()) return;
  
  LookupIdent();
  
  logs::Debug("Servicing client connected from %1%@%2%", ident, HostnameAndIP(LogAddresses::Normal));
    
  DisplayBanner();
  Handle();
}

void ClientImpl::Run()
{
  util::SetProcessTitle("CLIENT");
  logs::SetThreadIDPrefix('C' /* client */);
  
  auto finishedGuard = util::MakeScopeExit([&]
  {
    SetState(ClientState::Finished);
    std::make_shared<ftp::task::ClientFinished>(parent)->Push();
    if (user) db::mail::LogOffPurgeTrash(user->ID());
    LogTraffic();
  });

 try
  {
    InnerRun();
  }
  catch (const util::net::TimeoutError& e)
  {
    try
    {
      control.Reply(ftp::ServiceUnavailable, "Idle timeout exceeded, closing connection.");
    }
    catch (const util::net::NetworkError&) { }
    logs::Debug("Client from %1% connection timed out", control.RemoteEndpoint());
  }
  catch (const util::net::NetworkError& e)
  {
    logs::Debug("Client from %1% lost connection: %2%", control.RemoteEndpoint(), e.Message());
  }
/*  catch (const std::exception& e)
  {
    logs::Error("Unhandled error on client thread: %1%", e.what());
  }
  catch (...)
  {
    throw;
    logs::Error("Unhandled error on client thread: Not descended from std::exception");
  }*/
  
  (void) finishedGuard; /* silence unused variable warning */
}

CounterResult LoginGuard::Login(bool kicked, const boost::thread::id& tid)
{
  auto result = Counter::Login().Start(client.User().ID(), client.User().NumLogins(), 
                                       kicked, client.User().HasFlag(acl::Flag::Exempt));
  if (result != CounterResult::Okay) return result;

  OnlineWriter::Get().LoggedIn(tid, client, fs::WorkDirectory().ToString());
  
  this->tid = tid;
  loggedIn = true;
  return CounterResult::Okay;
}

void LoginGuard::Logout()
{
  assert(loggedIn);
  Counter::Login().Stop(client.User().ID());
  OnlineWriter::Get().LoggedOut(tid);  
  loggedIn = false;
}

} /* ftp namespace */
