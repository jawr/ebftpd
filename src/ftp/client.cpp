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

namespace ftp
{

std::atomic_bool Client::siteopOnly(false);

Client::Client() :
  data(*this), 
  user("root", 69, "password", "1"),
  state(ClientState::LoggedOut),
  passwordAttemps(0),
  ident("*"),
  kickLogin(false),
  idleTimeout(boost::posix_time::seconds(cfg::Get().IdleTimeout().Timeout()))
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
  currentCommand = args[0] + " " + argStr;
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

void Client::Handle()
{
  namespace pt = boost::posix_time;

  while (State() != ClientState::Finished)
  {
    if (State() != ClientState::LoggedIn || profile.IdleTime() == 0) 
      ExecuteCommand(control.NextCommand());
    else
    {
      pt::time_duration timeout = idleExpires - pt::second_clock::local_time();
      ExecuteCommand(control.NextCommand(&timeout));
    }
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

void Client::InnerRun()
{
  using util::scope_guard;
  using util::make_guard;
  
  scope_guard finishedGuard = make_guard([&]
  {
    SetState(ClientState::Finished);
    db::mail::LogOffPurgeTrash(user.UID());
    LogTraffic();
  });

  LookupIdent();
  
  logs::debug << "Servicing client connected from "
              << ident << "@" << control.RemoteEndpoint() << logs::endl;
    
  try
  {
    DisplayBanner();
    Handle();
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
  
  (void) finishedGuard; /* silence unused variable warning */
}

void Client::Run()
{
  try
  {
    InnerRun();
  }
  catch (const std::exception& e)
  {
    logs::error << "Unhandled error on client thread: " << e.what() << logs::endl;
  }
}

} /* ftp namespace */
