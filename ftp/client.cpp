#include <iomanip>
#include <boost/bind.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/trim.hpp>
#include "ftp/client.hpp"
#include "logger/logger.hpp"
#include "util/verify.hpp"
#include "util/error.hpp"
#include "util/scopeguard.hpp"
#include "util/net/tcplistener.hpp"
#include "cmd/factory.hpp"

namespace ftp
{

Client::~Client()
{
  control.Close();
  data.Close();
}

bool Client::IsFinished() const
{
  boost::lock_guard<boost::mutex> lock(mutex);
  return state == Finished;
}

void Client::SetLoggedIn()
{
  boost::lock_guard<boost::mutex> lock(mutex);
  state = LoggedIn;
}

void Client::SetWaitingPassword()
{
  boost::lock_guard<boost::mutex> lock(mutex);
  state = WaitingPassword;
}

void Client::SetLoggedOut()
{
  boost::lock_guard<boost::mutex> lock(mutex);
  state = LoggedOut;
}

void Client::SetFinished()
{
  boost::lock_guard<boost::mutex> lock(mutex);
  state = Finished;
}

bool Client::CheckState(ClientState reqdState)
{
  if (state == reqdState || reqdState == AnyState) return true;
  if (state == LoggedIn)
    Reply(ftp::NotLoggedIn, "Already logged in.");
  else if (state == WaitingPassword)
    Reply(ftp::BadCommandSequence, "Expecting PASS comamnd.");
  else if (state == LoggedOut && reqdState == WaitingPassword)
    Reply(ftp::BadCommandSequence, "Expecting USER command first.");
  else if (state == LoggedOut)
    Reply(ftp::NotLoggedIn, "Not logged in.");
  assert(state != Finished);
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
    server.Accept(control);
  }
  catch(const util::net::NetworkError& e)
  {
    SetFinished();
    logger::error << "Error while acceting new client: " << e.Message() << logger::endl;
    return false;
  }
  return true;
}

void Client::SendReply(ReplyCode code, bool part, const std::string& message)
{
  std::ostringstream reply;
  reply << std::setw(3) << code << (part ? "-" : " ") << message << "\r\n";
  const std::string& str = reply.str();
  control.Write(str.c_str(), str.length());
  lastCode = code;
}

void Client::PartReply(ReplyCode code, const std::string& message)
{
  SendReply(code, true, message);
}

void Client::PartReply(const std::string& message)
{
  verify(lastCode != CodeNotSet);
  PartReply(lastCode, message);
}

void Client::Reply(ReplyCode code, const std::string& message)
{
  std::ostringstream reply;
  SendReply(code, false, message);
  lastCode = CodeNotSet;
}

void Client::Reply(const std::string& message)
{
  verify(lastCode != CodeNotSet);
  Reply(lastCode, message);
}

void Client::MultiReply(ReplyCode code, const std::string& messages)
{
  std::vector<std::string> splitMessages;
  boost::split(splitMessages, messages, boost::is_any_of("\n"));
  assert(!splitMessages.empty());
  std::vector<std::string>::const_iterator end = splitMessages.end() - 1;
  for (std::vector<std::string>::const_iterator it =
       splitMessages.begin(); it != end; ++it)
  {
    PartReply(code, *it);
  }
  Reply(splitMessages.back());
}

void Client::DisplayWelcome()
{
  Reply(ftp::ServiceReady, "Welcome to eyeoh and biohazard's ftpd!");
}

void Client::NegotiateTLS()
{
  control.HandshakeTLS(util::net::TLSSocket::Server);
}

void Client::NextCommand()
{
  fd_set readSet;
  FD_ZERO(&readSet);
  FD_SET(control.Socket(), &readSet);
  FD_SET(interruptPipe[0], &readSet);
  
  struct timeval tv;
  tv.tv_sec = 1800;
  tv.tv_usec = 0;
  
  int max = std::max(control.Socket(), interruptPipe[0]);
  
  int n = select(max + 1, &readSet, NULL, NULL, &tv);
  if (!n) throw util::net::NetworkSystemError(ETIMEDOUT);
  if (n < 0) throw util::net::NetworkSystemError(errno);
  
  if (FD_ISSET(control.Socket(), &readSet))
  {
    control.Getline(commandLine, true);
    return;
  }

  boost::this_thread::interruption_point();
  verify(false); // should never get here!!
}

void Client::ExecuteCommand()
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
  
  ftp::ClientState reqdState;
  std::auto_ptr<cmd::Command>
    command(cmd::Factory::Create(*this, argStr, args, reqdState));
  if (!command.get()) Reply(ftp::CommandUnrecognised, "Command not understood");
  else if (!CheckState(reqdState));
  else command->Execute();
}

void Client::Handle()
{
  while (!IsFinished())
  {
    NextCommand();
    ExecuteCommand();
  }
}

void Client::Run()
{
  using util::scope_guard;
  using util::make_guard;
  using boost::bind;
  
  scope_guard finishedGuard = make_guard(bind(&Client::SetFinished, this));
  
  try
  {
    logger::ftpd << "Servicing client connected from "
                 << control.RemoteEndpoint() << logger::endl;
  
    DisplayWelcome();
    Handle();
  }
  catch (const util::net::NetworkError& e)
  {
    logger::error << "Client from " << control.RemoteEndpoint()
                  << " lost connection: " << e.Message() << logger::endl;
  }
  catch (const std::exception& e)
  {
    logger::error << e.what() << logger::endl;
  }
  
  (void) finishedGuard; /* silence unused variable warning */
}

void Client::DataListen()
{
  
}

void Client::DataConnect()
{
}


} /* ftp namespace */
