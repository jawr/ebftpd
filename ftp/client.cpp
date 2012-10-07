#include <iomanip>
#include <functional>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <cstdint>
#include <boost/optional.hpp>
#include "ftp/client.hpp"
#include "logger/logger.hpp"
#include "util/verify.hpp"
#include "util/error.hpp"
#include "util/scopeguard.hpp"
#include "util/net/tcplistener.hpp"
#include "cmd/factory.hpp"
#include "acl/check.hpp"
#include "cfg/get.hpp"
#include "util/misc.hpp"
#include "main.hpp"

namespace ftp
{

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

void Client::SetWaitingPassword()
{
  boost::lock_guard<boost::mutex> lock(mutex);
  state = ClientState::WaitingPassword;
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
  }
  catch(const util::net::NetworkError& e)
  {
    SetFinished();
    logger::error << "Error while accepting new client: " << e.Message() << logger::endl;
    return false;
  }
  return true;
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

void Client::ExecuteCommand(const std::string& commandLine)
{
  std::vector<std::string> args;
  boost::split(args, commandLine, boost::is_any_of(" "),
               boost::token_compress_on);
  if (args.empty()) throw util::net::NetworkError("FTP protocal violation");
  
  std::string argStr;
  if (args[0].length() < commandLine.length())
  {
    argStr = commandLine.substr(args[0].length() + 1);
    boost::trim(argStr);
  }
  
  ClientState reqdState;
  std::unique_ptr<cmd::Command>
    command(cmd::Factory::Create(*this, argStr, args, reqdState));
  if (!command.get()) control.Reply(ftp::CommandUnrecognised, "Command not understood");
  else if (!CheckState(reqdState));
  else command->Execute();
}

void Client::Handle()
{
  while (!IsFinished())
  {
    ExecuteCommand(control.NextCommand());
  }
}

void Client::Run()
{
  using util::scope_guard;
  using util::make_guard;
  
  scope_guard finishedGuard = make_guard(std::bind(&Client::SetFinished, this));

  try
  {
    logger::ftpd << "Servicing client connected from "
                 << control.RemoteEndpoint() << logger::endl;
  
    DisplayBanner();
    Handle();
  }
  catch (const util::net::NetworkError& e)
  {
    logger::error << "Client from " << control.RemoteEndpoint()
                  << " lost connection: " << e.Message() << logger::endl;
  }
  /*catch (const std::exception& e)
  {
    logger::error << e.what() << logger::endl;
  }*/
  
  (void) finishedGuard; /* silence unused variable warning */
}

} /* ftp namespace */
