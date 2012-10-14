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
#include "acl/check.hpp"
#include "cfg/get.hpp"
#include "util/misc.hpp"
#include "acl/ipmaskcache.hpp"
#include "main.hpp"
#include "util/net/identclient.hpp"

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

bool Client::IsFinished() const
{
  boost::lock_guard<boost::mutex> lock(mutex);
  return state == ClientState::Finished;
}

void Client::SetLoggedIn()
{
  boost::lock_guard<boost::mutex> lock(mutex);
  state = ClientState::LoggedIn;
}

void Client::SetWaitingPassword(const acl::User& user)
{
  {
    boost::lock_guard<boost::mutex> lock(mutex);
    state = ClientState::WaitingPassword;
  }
  
  this->user = user;
}

void Client::SetLoggedOut()
{
  boost::lock_guard<boost::mutex> lock(mutex);
  state = ClientState::LoggedOut;
}

void Client::SetFinished()
{
  boost::lock_guard<boost::mutex> lock(mutex);
  state = ClientState::Finished;
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
    SetFinished();
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
      
  idleExpires = boost::posix_time::second_clock::local_time() + idleTimeout;
}

void Client::ExecuteCommand(const std::string& commandLine)
{
  std::vector<std::string> args;
  boost::split(args, commandLine, boost::is_any_of(" "),
               boost::token_compress_on);
  if (args.empty()) throw util::net::NetworkError("FTP protocal violation");
  
  std::string argStr(commandLine.substr(args[0].length()));
  boost::trim(argStr);
  
  ClientState reqdState;
  std::unique_ptr<cmd::Command>
    command(cmd::rfc::Factory::Create(*this, argStr, args, reqdState));
  if (!command.get()) control.Reply(ftp::CommandUnrecognised, "Command not understood");
  else if (!CheckState(reqdState));
  else command->Execute();
  IdleReset(commandLine);
}

void Client::Handle()
{
  namespace pt = boost::posix_time;

  while (!IsFinished())
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

void Client::Run()
{
  using util::scope_guard;
  using util::make_guard;
  
  scope_guard finishedGuard = make_guard([this]{ SetFinished(); });

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
