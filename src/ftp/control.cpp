#include <iomanip>
#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>
#include "ftp/control.hpp"
#include "util/verify.hpp"
#include "util/net/tcplistener.hpp"
#include "logs/logs.hpp"

namespace ftp
{

void Control::Accept(util::net::TCPListener& listener)
{
  listener.Accept(socket);
}


void Control::SendReply(ReplyCode code, bool part, const std::string& message)
{
  std::ostringstream reply;
  if (code != NoCode)
    reply << std::setw(3) << code << (part ? "-" : " ");
  reply << message << "\r\n";
  const std::string& str = reply.str();
  socket.Write(str.c_str(), str.length());
  logs::debug << str << logs::endl;
  lastCode = code;
}

void Control::PartReply(ReplyCode code, const std::string& message)
{
  SendReply(code, true, message);
}

void Control::PartReply(const std::string& message)
{
  verify(lastCode != CodeNotSet);
  PartReply(lastCode, message);
}

void Control::Reply(ReplyCode code, const std::string& message)
{
  std::ostringstream reply;
  SendReply(code, false, message);
  lastCode = CodeNotSet;
}

void Control::Reply(const std::string& message)
{
  verify(lastCode != CodeNotSet);
  Reply(lastCode, message);
}

void Control::MultiReply(ReplyCode code, const std::string& messages)
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
  Reply(code, splitMessages.back());
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

    n = select(max + 1, &readSet, NULL, NULL, &tv);
  }
  else
    n = select(max + 1, &readSet, NULL, NULL, NULL);
    
  if (!n) throw util::net::TimeoutError();
  if (n < 0) throw util::net::NetworkSystemError(errno);
  
  if (FD_ISSET(socket.Socket(), &readSet))
  {
    std::string commandLine;
    socket.Getline(commandLine, true);
    logs::debug << commandLine << logs::endl;
    return commandLine;
  }

  boost::this_thread::interruption_point();
  verify(false); // should never get here!!
  return "";
}

} /* ftp namespace */
