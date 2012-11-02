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

namespace ftp
{

Client::Client() :
  data(*this), 
  workDir("/"), 
  user("root", 69, "password", "1"),
  state(ClientState::LoggedOut),
  passwordAttemps(0),
  ident("*"),
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
  boost::lock_guard<boost::mutex> lock(mutex);
  this->state = state;
}

void Client::SetLoggedIn()
{
  boost::lock_guard<boost::mutex> lock(mutex);
  state = ClientState::LoggedIn;
  loggedInAt = boost::posix_time::second_clock::local_time();
  db::user::Login(user.UID());
  try
  {
    profile = db::userprofile::Get(user.UID());
  }
  catch (const util::RuntimeError& e)
  {
    logs::error << "Error loading UserProfile for " << user.Name() << logs::endl;
    // do we make a blank profile here?
  }
}

void Client::SetWaitingPassword(const acl::User& user)
{
  {
    boost::lock_guard<boost::mutex> lock(mutex);
    state = ClientState::WaitingPassword;
  }
  
  this->user = user;
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

void Client::SetWorkDir(const fs::Path& workDir)
{
  this->workDir = workDir;
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
  if (!config.Banner().Empty())
  {
    std::string banner;
    if (util::ReadFileToString(config.Banner(), banner))
    {
      control.MultiReply(ftp::ServiceReady, banner);
      return;
    }
  }

  control.Reply(ftp::ServiceReady, config.SitenameLong() + ": " + programFullname + 
        " connected.");
}

void Client::IdleReset(const std::string& commandLine)
{
  for (auto & cmd : cfg::Get().IdleCommands())
    if (boost::starts_with(commandLine, cmd) &&
        (commandLine.length() == cmd.length() ||
         (commandLine.length() >= cmd.length() &&
          commandLine[cmd.length()] == ' ')))
      return;
  idleTime = boost::posix_time::second_clock::local_time();
  idleExpires = idleTime + idleTimeout;
}

void Client::ExecuteCommand(const std::string& commandLine)
{
  std::vector<std::string> args;
  boost::split(args, commandLine, boost::is_any_of(" "),
               boost::token_compress_on);
  if (args.empty()) throw util::net::NetworkError("FTP protocal violation");
  
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
  else if (CheckState(def->RequiredState()))
  {
    cmd::CommandPtr command(def->Create(*this, argStr, args));
    if (!command)
    {
      control.Reply(ftp::NotImplemented, "Command not implemented");
    }
    else
    {
      if (command->Execute() == cmd::Result::SyntaxError)
      {
        control.Reply(ftp::SyntaxError, "Syntax: " + def->Syntax());
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
    pt::time_duration timeout = idleExpires - pt::second_clock::local_time();
    ExecuteCommand(control.NextCommand(timeout));
  }
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

void Client::Run()
{
  using util::scope_guard;
  using util::make_guard;
  
  scope_guard finishedGuard = make_guard([&]{ SetState(ClientState::Finished); });

  LookupIdent();
  
  logs::debug << "Servicing client connected from "
              << ident << "@" << control.RemoteEndpoint() << logs::endl;
    
  try
  {
    DisplayBanner();
    Handle();
  }
  catch (const util::net::NetworkError& e)
  {
    logs::debug << "Client from " << control.RemoteEndpoint()
                  << " lost connection: " << e.Message() << logs::endl;
  }
  
  (void) finishedGuard; /* silence unused variable warning */
}

} /* ftp namespace */
