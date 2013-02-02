#include <iomanip>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include "ftp/control.hpp"
#include "util/verify.hpp"
#include "util/net/tcplistener.hpp"
#include "logs/logs.hpp"
#include "ftp/error.hpp"
#include "ftp/util.hpp"

namespace ftp
{

void Control::Accept(util::net::TCPListener& listener)
{
  listener.Accept(socket);
}

void Control::SendReply(ReplyCode code, bool part, const std::string& message)
{
  if (singleLineReplies && part) return;
  
  std::ostringstream reply;
  if (code != NoCode) reply << std::setw(3) << code << (part ? "-" : " ");
  reply << message << "\r\n";
  
  const std::string& str = reply.str();
  Write(str.c_str(), str.length());
  logs::debug << str << logs::endl;
  
  if (lastCode != code && lastCode != CodeNotSet && code != ftp::NoCode)
    throw ProtocolError("Invalid reply code sequence.");
  if (code != ftp::NoCode) lastCode = code;
}

void Control::PartReply(ReplyCode code, const std::string& messages)
{
  assert(code != CodeNotSet);
  
  if (code == ftp::CodeDeferred)
  {
    std::vector<std::string> splitMessages;
    boost::split(splitMessages, messages, boost::is_any_of("\n"));
    deferred.insert(deferred.end(), splitMessages.begin(), splitMessages.end());
  }
  else
    MultiReply(code, false, messages);
}

void Control::Reply(ReplyCode code, const std::string& messages)
{
  assert(code != CodeNotSet && code != CodeDeferred);
  if (!deferred.empty())
  {
    MultiReply(code, false, deferred);
    deferred.clear();
  }
  
  MultiReply(code, true, messages);
  lastCode = CodeNotSet;
}

void Control::MultiReply(ReplyCode code, bool final, const std::vector<std::string>& messages)
{
  assert(!messages.empty());
  std::vector<std::string>::const_iterator end = messages.end() - 1;
  for (auto it = messages.begin(); it != end; ++it)
  {
    SendReply(code, true, *it);
  }
  SendReply(code, !final, messages.back());
}

void Control::MultiReply(ReplyCode code, bool final, const std::string& messages)
{
  std::vector<std::string> splitMessages;
  boost::split(splitMessages, messages, boost::is_any_of("\n"));
  assert(!splitMessages.empty());
  MultiReply(code, final, splitMessages);
}

void Control::NegotiateTLS()
{
  socket.HandshakeTLS(util::net::TLSSocket::Server);
}

std::string Control::NextCommand(const boost::posix_time::time_duration* timeout)
{
  fd_set readSet;
  FD_ZERO(&readSet);
  FD_SET(socket.Socket(), &readSet);
  FD_SET(interruptPipe.ReadFd(), &readSet);
    
  int max = std::max(socket.Socket(), interruptPipe.ReadFd());

  int n;
  if (timeout)
  {
    struct timeval tv;
    tv.tv_sec = std::max(0, timeout->total_seconds());
    tv.tv_usec = 0;

    n = select(max + 1, &readSet, nullptr, nullptr, &tv);
  }
  else
    n = select(max + 1, &readSet, nullptr, nullptr, nullptr);

  boost::this_thread::interruption_point();
    
  if (!n) throw util::net::TimeoutError();
  if (n < 0) throw util::net::NetworkSystemError(errno);
  
  if (FD_ISSET(socket.Socket(), &readSet))
  {
    std::string commandLine;
    socket.Getline(commandLine, false);
    bytesRead += commandLine.length();
    boost::trim_right_if(commandLine, boost::is_any_of("\n"));
    boost::trim_right_if(commandLine, boost::is_any_of("\r"));
    StripTelnetChars(commandLine);
    logs::debug << commandLine << logs::endl;    
    return commandLine;
  }

  verify(false); // should never get here!!
  return "";
}

std::string Control::WaitForIdnt()
{
  try
  {
    boost::posix_time::seconds timeout(1);
    return NextCommand(&timeout);
  }
  catch (const util::net::TimeoutError&)
  {
    return std::string("");
  }
}

} /* ftp namespace */
