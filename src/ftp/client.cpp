#include <iomanip>
#include <functional>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <cstdint>
#include <boost/optional.hpp>
#include "ftp/client.hpp"
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
#include "acl/ipmaskcache.hpp"
#include "main.hpp"
#include "util/net/identclient.hpp"
#include "util/string.hpp"
#include "db/user/user.hpp"
#include "db/user/userprofile.hpp"
#include "ftp/counter.hpp"
#include "acl/flags.hpp"
#include "db/mail/mail.hpp"
#include "db/stats/protocol.hpp"
#include "ftp/error.hpp"
#include "cmd/error.hpp"
#include "exec/cscript.hpp"
#include "acl/ipmaskcache.hpp"
#include "util/net/resolver.hpp"
#include "acl/usercache.hpp"
#include "db/error.hpp"

namespace ftp
{

std::atomic_bool Client::siteopOnly(false);

Client::Client() :
  data(*this), 
  user("root", 69, "password", "1"),
  state(ClientState::LoggedOut),
  passwordAttemps(0),
  kickLogin(false),
  idleTimeout(boost::posix_time::seconds(cfg::Get().IdleTimeout().Timeout())),
  ident("*")
{
}

Client::~Client()
{
}

void Client::SetState(ClientState state)
{
  assert(state != ClientState::LoggedIn); // these 2 states have own setter
  assert(state != ClientState::WaitingPassword);
  
  if (state == ClientState::Finished) Counter::LogOut(user.UID());
  
  boost::lock_guard<boost::mutex> lock(mutex);
  this->state = state;
}

void Client::SetLoggedIn(const acl::UserProfile& profile, bool kicked)
{
  
  util::Error e(Counter::LogIn(user.UID(), profile.NumLogins(), kicked, 
                      user.CheckFlag(acl::Flag::Exempt)));
  if (!e) throw util::RuntimeError(e.Message());

  if (profile.IdleTime() == -1)
    SetIdleTimeout(cfg::Get().IdleTimeout().Timeout());
  else
    SetIdleTimeout(boost::posix_time::seconds(profile.IdleTime()));
  
  boost::lock_guard<boost::mutex> lock(mutex);
  this->profile = std::move(profile);
  state = ClientState::LoggedIn;
  loggedInAt = boost::posix_time::second_clock::local_time();
}

void Client::SetWaitingPassword(const acl::User& user, bool kickLogin)
{
  {
    boost::lock_guard<boost::mutex> lock(mutex);
    state = ClientState::WaitingPassword;
    this->user = std::move(user);
    this->kickLogin = kickLogin;
  }
}

bool Client::CheckState(ClientState reqdState)
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

bool Client::VerifyPassword(const std::string& password)
{
  ++passwordAttemps;
  return user.VerifyPassword(password);
}

bool Client::PasswordAttemptsExceeded() const
{
  return passwordAttemps >= maxPasswordAttemps;
}

bool Client::Accept(util::net::TCPListener& server)
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
    logs::error << "Error while accepting new client: " << e.Message() << logs::endl;
    return false;
  }
}

void Client::DisplayBanner()
{
  const cfg::Config& config = cfg::Get();
  if (!config.Banner().IsEmpty())
  {
    std::string banner;
    if (util::ReadFileToString(config.Banner().ToString(), banner))
    {
      control.Reply(ftp::ServiceReady, banner);
      return;
    }
  }

  control.Reply(ftp::ServiceReady, config.LoginPrompt());
}

void Client::IdleReset(std::string commandLine)
{
  for (auto & mask : cfg::Get().IdleCommands())
    if (util::string::WildcardMatch(mask, commandLine, true))
      return;
  idleTime = boost::posix_time::second_clock::local_time();
  idleExpires = idleTime + idleTimeout;
}

void Client::ExecuteCommand(const std::string& commandLine)
{
  std::vector<std::string> args;
  boost::split(args, commandLine, boost::is_any_of(" "),
               boost::token_compress_on);
  if (args.empty()) throw ProtocolError("Empty command.");
  
  std::string argStr(commandLine.substr(args[0].length()));
  boost::trim(argStr);
  boost::to_upper(args[0]);
  
  currentCommand = args[0];
  if (!argStr.empty()) currentCommand += " " + argStr;
  
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
           exec::Cscripts(*this, args[0], currentCommand, exec::CscriptType::PRE, 
                def->FailCode()))
  {
    cmd::CommandPtr command(def->Create(*this, argStr, args));
    if (!command)
    {
      control.Reply(ftp::NotImplemented, "Command not implemented");
    }
    else
    {
      try
      {
        command->Execute();
        exec::Cscripts(*this, args[0], currentCommand, exec::CscriptType::POST, 
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
}

void Client::ReloadUser()
{
  userUpdated = false;

  try
  {
    {
      boost::lock_guard<boost::mutex>  lock(mutex);
      user = acl::UserCache::User(user.Name());
    }    
  }
  catch (const util::RuntimeError& e)
  {
    logs::error << "Failed to reload user from cache for: " 
                << user.Name() << logs::endl;
  }  
  
  try
  {
    acl::UserProfile profile(db::userprofile::Get(user.UID()));
  
    boost::lock_guard<boost::mutex> lock(mutex);
    this->profile = std::move(profile);
  }
  catch (const db::DBError& e)
  {
    logs::error << "Failed to reload user profile from cache for: " 
                << user.Name() << logs::endl;
  }
}

void Client::Handle()
{
  namespace pt = boost::posix_time;

  pt::time_duration timeout;
  pt::time_duration* timeoutPtr = nullptr;
  
  while (State() != ClientState::Finished)
  {
    if (State() != ClientState::LoggedIn || profile.IdleTime() == 0) 
      timeoutPtr = nullptr;
    else
    {     
      timeout = idleExpires - pt::second_clock::local_time();
      timeoutPtr = &timeout;
    }
    
    std::string command = control.NextCommand(timeoutPtr);
    if (userUpdated) ReloadUser();
    ExecuteCommand(command);
    cfg::UpdateLocal();
  }
}

void Client::Interrupt()
{
  SetState(ClientState::Finished);
  Stop();
  control.Interrupt();
  data.Interrupt();
  child.Interrupt();
}

void Client::LookupIdent()
{
  if (ident != "*") return;
  
  try
  {
    util::net::IdentClient identClient(control.LocalEndpoint(), 
                                       control.RemoteEndpoint());
    ident = identClient.Ident();
  }
  catch (util::net::NetworkError& e)
  {
    logs::error << "Unable to lookup ident for connection from "
                  << control.RemoteEndpoint() << ":  " << e.Message();
  }
}

bool Client::ConfirmCommand(const std::string& argStr)
{
  std::string command = 
      util::string::CompressWhitespaceCopy(argStr);
  if (command != confirmCommand)
  {
    confirmCommand = command;
    return false;
  }
  confirmCommand.clear();
  return true;
}

void Client::LogTraffic() const
{
  db::stats::ProtocolUpdate(user.UID(), control.BytesWrite() + data.BytesWrite(),
        control.BytesRead() + data.BytesRead());
}

bool Client::PostCheckAddress()
{
  return acl::IpMaskCache::Check(user.UID(), ident + "@" + IP()) ||
        (IP() != Hostname() && acl::IpMaskCache::Check(user.UID(), ident + "@" + Hostname()));
}

bool Client::PreCheckAddress()
{
  if (!acl::IpMaskCache::Check("*@" + IP()) ||
        (IP() != Hostname() && acl::IpMaskCache::Check("*@" + Hostname())))
  {
    logs::security << "Refused connection from unknown address: " 
                   << HostnameAndIP() << logs::endl;
    return false;
  }
  
  return true;
}

void Client::HostnameLookup()
{
  if (!hostname.empty()) return;
  
  try
  {
  
    std::string hostname = util::net::ReverseResolve(util::net::IPAddress(ip));
    
    {
      boost::lock_guard<boost::mutex> lock(mutex);
      this->hostname = hostname;
    }
  }
  catch (const util::net::NetworkError&)
  {
    {
      boost::lock_guard<boost::mutex> lock(mutex);
      this->hostname = ip;
    }
  }
}

std::string Client::HostnameAndIP() const
{
  boost::lock_guard<boost::mutex> lock(mutex);
  std::ostringstream os;
  os << hostname;
  if (ip != hostname)
  os << "(" << ip << ")";
  return os.str();
}

bool Client::IdntUpdate(const std::string& ident, std::string ip,
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
    boost::lock_guard<boost::mutex> lock(mutex);
    this->ident = ident;
    this->ip = ip;
    if (ip != hostname) this->hostname = hostname;
  }
  
  return true;
}

bool Client::IdntParse(const std::string& command)
{
  std::vector<std::string> args;
  boost::split(args, command, boost::is_any_of(" "));
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

void Client::InnerRun()
{
  if (!cfg::Get().IsBouncer(ip))
  {
    if (cfg::Get().BouncerOnly() && !control.RemoteEndpoint().IP().IsLoopback())
    {
      logs::security << "Refused connection not from a bouncer ip: " 
                     << HostnameAndIP() << logs::endl;
      return;
    }
  }
  else
  {
    std::string command = control.WaitForIdnt();
    if (command.empty() && cfg::Get().BouncerOnly())
    {
      logs::security << "Timeout while waiting for IDNT command from bouncer: "
                     << HostnameAndIP() << logs::endl;
      return;
    }
    
    if (!IdntParse(command))
    {
      logs::security << "Malformed IDNT Command from bouncer: "
                     << HostnameAndIP() << logs::endl;
      return;
    }
  }

  if (!PreCheckAddress()) return;
  
  HostnameLookup();
  LookupIdent();
  
  logs::debug << "Servicing client connected from "
              << ident << "@" << HostnameAndIP() << logs::endl;
    
  DisplayBanner();
  Handle();
}

void Client::Run()
{
  using util::scope_guard;
  using util::make_guard;
  
  scope_guard finishedGuard = make_guard([&]
  {
    SetState(ClientState::Finished);
    if (user.UID() != -1) db::mail::LogOffPurgeTrash(user.UID());
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
    logs::debug << "Client from " << control.RemoteEndpoint()
                  << " connection timed out" << logs::endl;
  }
  catch (const util::net::NetworkError& e)
  {
    logs::debug << "Client from " << control.RemoteEndpoint()
                << " lost connection: " << e.Message() << logs::endl;
  }
/*  catch (const std::exception& e)
  {
    logs::error << "Unhandled error on client thread: " << e.what() << logs::endl;
  }
  catch (...)
  {
    throw;
    logs::error << "Unhandled error on client thread: Not descended from std::exception" << logs::endl;
  }*/
  
  (void) finishedGuard; /* silence unused variable warning */
}

} /* ftp namespace */
