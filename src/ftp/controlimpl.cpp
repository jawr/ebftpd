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

#include <csignal>
#include <iomanip>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <poll.h>
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
  sigset_t mask;
  sigfillset(&mask);
  sigdelset(&mask, SIGUSR1);
  
  struct pollfd fds[1];
  fds[0].fd = socket.Socket();
  fds[0].events = POLLIN;
  fds[0].revents = 0;

  int pollTimeout = !timeout ? -1 : timeout->total_milliseconds();
  
  int n = poll(fds, 1, pollTimeout);
  if (!n)
  {
    throw util::net::TimeoutError();
  }
  else
  if (n < 0)
  {
    if (errno == EINTR)
    {
      boost::this_thread::interruption_point();
      verify(false);
    }
    else
    {
     throw util::net::NetworkSystemError(errno);
    }
  }

  if (fds[0].revents & POLLIN)
  {
    std::string commandLine;
    socket.Getline(commandLine, false);
    bytesRead += commandLine.length();
    util::TrimRightIf(commandLine, "\n");
    util::TrimRightIf(commandLine, "\r");
    StripTelnetChars(commandLine);
    logs::Debug(commandLine);
    return commandLine;
  }

  if (fds[0].revents & POLLHUP) throw util::net::EndOfStream();
  throw util::net::NetworkError();
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
