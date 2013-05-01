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

#include <boost/thread/thread.hpp>
#include <sys/select.h>
#include <poll.h>
#include "ftp/portallocator.hpp"
#include "ftp/addrallocator.hpp"
#include "util/net/interfaces.hpp"
#include "ftp/data.hpp"
#include "logs/logs.hpp"
#include "acl/misc.hpp"
#include "acl/user.hpp"
#include "ftp/client.hpp"
#include "cfg/get.hpp"
#include "ftp/error.hpp"
#include "ftp/control.hpp"
#include "util/verify.hpp"

namespace util
{

template <> const char* util::EnumStrings<ftp::EPSVMode>::values[] =
{
  "EXTENDED",
  "NORMAL"
};

}

namespace ftp
{

Data::Data(Client& client) :
  client(client),
  protection(false),
  pasvType(PassiveType::None),
  epsvMode(cfg::Get().EPSVFxp() == ::cfg::EPSVFxp::Force ? 
           ::ftp::EPSVMode::Extended : 
           ::ftp::EPSVMode::Normal),
  dataType(::ftp::DataType::Binary),
  sscnMode(::ftp::SSCNMode::Server),
  restartOffset(0),
  bytesRead(0),
  bytesWrite(0)
{
}

void Data::InitPassive(util::net::Endpoint& ep, PassiveType pasvType)
{
  using namespace util::net;

  socket.Close();
  listener.Close();
  
  boost::optional<util::net::IPAddress> ip;
  
  // unable to use alternative pasv_addr if espv mode isn't Full
  // should we fail -- or substitute the ip 
  // from the client.Control.LocalEndpoint like now?
  if (pasvType != PassiveType::EPSV || epsvMode == ::ftp::EPSVMode::Extended)
  {
    std::string firstAddr;
    while (true)
    {
      std::string addr = AddrAllocator<AddrType::Passive>::Get().NextAddr();
      if (addr.empty()) break;
      
      if (addr == firstAddr)
        throw NetworkError("Unable to find a valid local address.");
      
      try
      {
        ip.reset(IPAddress(addr));
        if (pasvType == PassiveType::PASV && ip->Family() == IPFamily::IPv4)
          break;
        else
        if (ip->Family() == client.Control().LocalEndpoint().IP().Family())
          break;
      }
      catch (const NetworkError&)
      {
      }
      
      if (firstAddr.empty()) firstAddr = addr;
    }
  }
  
  if (!ip) ip = IPAddress(client.Control().LocalEndpoint().IP());
  if (pasvType == PassiveType::PASV && ip->Family() == IPFamily::IPv6)
    FindPartnerIP(*ip, *ip);

  boost::optional<int> firstPort;
  while (true)
  {
    int port = PortAllocator<PortType::Passive>::Get().NextPort();
    if (!firstPort) firstPort.reset(port);
    else if (port == *firstPort) 
      throw util::net::NetworkError("All ports exhausted.");
      
    try
    {
      listener.Listen(Endpoint(*ip, port));
      break;
    }
    catch (const util::net::NetworkSystemError& e)
    {
      if (e.Errno() != EADDRINUSE)
        throw;
    }
  }

  this->pasvType = pasvType;
  
  const std::string& natAddr = cfg::Get().NATAddr();
  if (!natAddr.empty())
  {
    ep = Endpoint(IPAddress(natAddr), listener.Endpoint().Port());
  }
  else
  {
    ep = listener.Endpoint();
  }
}

void Data::InitActive(const util::net::Endpoint& ep)
{
  pasvType = PassiveType::None;
  socket.Close();
  listener.Close();
  
  boost::optional<util::net::IPAddress> localIP;
  std::string firstAddr;
  while (true)
  {
    std::string addr = AddrAllocator<AddrType::Active>::Get().NextAddr();
    if (addr.empty()) break;
    
    if (addr == firstAddr)
      throw util::net::NetworkError("Unable to find a valid local address.");
    
    try
    {
      localIP.reset(util::net::IPAddress(addr));
      if (localIP->Family() == ep.Family()) break;
    }
    catch (const util::net::NetworkError&)
    {
    }
    
    if (firstAddr.empty()) firstAddr = addr;
  }
  
  if (!localIP) localIP = util::net::IPAddress(ep.Family());
  
  boost::optional<int> firstPort;
  while (true)
  {
    int localPort = PortAllocator<PortType::Active>::Get().NextPort();
    if (!firstPort) firstPort.reset(localPort);
    else if (localPort == *firstPort) 
      throw util::net::NetworkError("All ports exhausted.");
      
    try
    {
      socket.Connect(ep, util::net::Endpoint(*localIP, localPort));
      break;
    }
    catch (const util::net::NetworkSystemError& e)
    {
      if (e.Errno() != EADDRINUSE)
        throw;
    }
  }
}

void Data::Open(TransferType transferType)
{
  if (pasvType != PassiveType::None)
  {
    assert(listener.IsListening());
    listener.Accept(socket);
  }
  else
  if (!socket.IsConnected())
  {
    throw util::net::NetworkError("Invalid command sequence");
  }
  
  if (transferType != TransferType::List && IsFXP())
  {
    bool logging;
    
    if (transferType == TransferType::Upload ?
        !acl::AllowFxpReceive(client.User(), logging) :
        !acl::AllowFxpSend(client.User(), logging))
    {
      socket.Close();
      std::string type = transferType == TransferType::Upload ? "upload from" : "download to";
      if (logging)
      { 
        logs::Security("FXP", "'%1%' attempted to fxp %2% '%3%'", client.User().Name(), socket.RemoteEndpoint());
      }
      
      throw util::net::NetworkError("FXP " + type + " not allowed.");
    }
  }
  
  if (protection)
  {
    util::net::TLSSocket::HandshakeRole role = util::net::TLSSocket::Server;
    if ((sscnMode == ftp::SSCNMode::Client || pasvType == PassiveType::CPSV) && 
        (transferType == TransferType::Upload || transferType == TransferType::Download))
    {
      role = util::net::TLSSocket::Client;  
    }
    
    socket.HandshakeTLS(role);
  }
  
  state.Start(transferType);
}

bool Data::IsFXP() const
{
  return socket.RemoteEndpoint().IP() != client.Control().RemoteEndpoint().IP();
}

bool Data::ProtectionOkay() const
{
  if (protection) return true;
  
  if (state.Type() == TransferType::List)
  {
    return !cfg::Get().TLSListing().Evaluate(client.User().ACLInfo());
  }
  else
  if (IsFXP())
  {
    return !cfg::Get().TLSFxp().Evaluate(client.User().ACLInfo());
  }

  return !cfg::Get().TLSData().Evaluate(client.User().ACLInfo());
}

void Data::HandleControl(int revents)
{
  try
  {
    if (revents & POLLIN)
    {
      std::string command = client.Control().NextCommand();
      
      if (command == "ABOR")
      {
        client.Control().Reply(ftp::DataCloseAborted, 
            "Abort requested, closing data connection");
        throw TransferAborted();
      }
      else
      if (command == "QUIT")
      {
        client.Control().Reply(ftp::ClosingControl, "Bye bye");
        client.SetState(ftp::ClientState::Finished);
        throw util::net::EndOfStream();
      }
      else
      if (command == "STAT")
      {
        std::ostringstream os;
        os << "Status: " << state.Bytes() << " bytes transferred";
        client.Control().Reply(ftp::FileStatus, os.str());
      }
      else
      {
        client.Control().Reply(ftp::BadCommandSequence, 
                  "Unsupported command during transfer");
      }
    }
    
    if (revents & POLLHUP) throw util::net::EndOfStream();
    throw util::net::NetworkError();
  }
  catch (const util::net::NetworkError& e)
  {
    throw ftp::ControlError(std::current_exception());
  }
}

size_t Data::Read(char* buffer, size_t size)
{
  int pollTimeout = (socket.Timeout().Seconds() * 1000 ) + 
                    (socket.Timeout().Microseconds() / 1000);
  struct pollfd fds[2];
  
  fds[0].fd = client.Control().socket->Socket();
  fds[0].events = POLLIN;
  
  fds[1].fd = socket.Socket();
  fds[1].events = POLLIN;
  
  while (true)
  {
    fds[0].revents = 0;
    fds[1].revents = 0;
    
    int n = poll(fds, 2, pollTimeout);
    if (!n) throw util::net::TimeoutError();
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
    
    if (fds[0].revents > 0) HandleControl(fds[0].revents);
    if (fds[1].revents & POLLIN) return socket.Read(buffer, size);
    if (fds[1].revents & POLLHUP) throw util::net::EndOfStream();
    throw util::net::NetworkError();
  }
}

void Data::Write(const char* buffer, size_t len)
{
  int pollTimeout = (socket.Timeout().Seconds() * 1000 ) + 
                    (socket.Timeout().Microseconds() / 1000);
  struct pollfd fds[2];
  
  fds[0].fd = client.Control().socket->Socket();
  fds[0].events = POLLIN;
  
  fds[1].fd = socket.Socket();
  fds[1].events = POLLOUT;
  
  while (true)
  {
    fds[0].revents = 0;
    fds[1].revents = 0;
    
    int n = poll(fds, 2, pollTimeout);
    if (!n) throw util::net::TimeoutError();
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
    
    if (fds[0].revents > 0) HandleControl(fds[0].revents);
    if (fds[1].revents & POLLOUT)
    {
      socket.Write(buffer, len);
      if (state.Type() == TransferType::List)
        bytesWrite += len;
      return;
    }
    if (fds[1].revents & POLLHUP) throw util::net::EndOfStream();
    throw util::net::NetworkError();
  }
}

void Data::Interrupt()
{
  socket.Shutdown();
  listener.Shutdown();
}


} /* ftp namespace */
