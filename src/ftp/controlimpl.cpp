#include <iomanip>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include "ftp/controlimpl.hpp"
#include "util/string.hpp"
#include "util/verify.hpp"
#include "util/net/tcplistener.hpp"
#include "logs/logs.hpp"
#include "ftp/error.hpp"
#include "ftp/util.hpp"

namespace ftp
{

ControlImpl::ControlImpl(util::net::TCPSocket** socket) : 
  lastCode(CodeNotSet), 
  singleLineReplies(false), 
  bytesRead(0), 
  bytesWrite(0)
{
  *socket = &this->socket;
}

void ControlImpl::Accept(util::net::TCPListener& listener)
{
  listener.Accept(socket);
}

void ControlImpl::SendReply(ReplyCode code, bool part, const std::string& message)
{
  if (singleLineReplies && part) return;
  
  std::ostringstream reply;
  if (code != NoCode) reply << std::setw(3) << code << (part ? "-" : " ");
  reply << message;
  logs::Debug(reply.str());
  reply << "\r\n";
  
  const std::string& str = reply.str();
  Write(str.c_str(), str.length());

  if (lastCode != code && lastCode != CodeNotSet && code != ftp::NoCode)
    throw ProtocolError("Invalid reply code sequence.");
  if (code != ftp::NoCode) lastCode = code;
}

void ControlImpl::PartReply(ReplyCode code, const std::string& messages)
{
  assert(code != CodeNotSet);
  
  if (code == ftp::CodeDeferred)
  {
    std::vector<std::string> splitMessages;
    util::Split(splitMessages, messages, "\n");
    deferred.insert(deferred.end(), splitMessages.begin(), splitMessages.end());
  }
  else
    MultiReply(code, false, messages);
}

void ControlImpl::Reply(ReplyCode code, const std::string& messages)
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

void ControlImpl::MultiReply(ReplyCode code, bool final, const std::vector<std::string>& messages)
{
  assert(!messages.empty());
  std::vector<std::string>::const_iterator end = messages.end() - 1;
  for (auto it = messages.begin(); it != end; ++it)
  {
    SendReply(code, true, *it);
  }
  SendReply(code, !final, messages.back());
}

void ControlImpl::MultiReply(ReplyCode code, bool final, const std::string& messages)
{
  std::vector<std::string> splitMessages;
  util::Split(splitMessages, messages, "\n");
  assert(!splitMessages.empty());
  MultiReply(code, final, splitMessages);
}

void ControlImpl::NegotiateTLS()
{
  socket.HandshakeTLS(util::net::TLSSocket::Server);
}

std::string ControlImpl::NextCommand(const boost::posix_time::time_duration* timeout)
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
  
  std::string commandLine;
  socket.Getline(commandLine, false);
  bytesRead += commandLine.length();
  util::TrimRightIf(commandLine, "\n");
  util::TrimRightIf(commandLine, "\r");
  StripTelnetChars(commandLine);
  logs::Debug(commandLine);
  return commandLine;
}

std::string ControlImpl::WaitForIdnt()
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
