#include <sys/select.h>
#include "ftp/portallocator.hpp"
#include "ftp/addrallocator.hpp"
#include "util/net/interfaces.hpp"
#include "ftp/data.hpp"
#include "logs/logs.hpp"
#include "acl/allowfxp.hpp"
#include "acl/user.hpp"
#include "ftp/client.hpp"
#include "cfg/get.hpp"
#include "ftp/error.hpp"

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
      std::string addr = AddrAllocator<AddrType::Passive>::NextAddr();
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
  
  boost::optional<uint16_t> firstPort;
  while (true)
  {
    uint16_t port = PortAllocator<PortType::Passive>::NextPort();
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
  ep = listener.Endpoint();
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
    std::string addr = AddrAllocator<AddrType::Active>::NextAddr();
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
  
  boost::optional<uint16_t> firstPort;
  while (true)
  {
    uint16_t localPort = PortAllocator<PortType::Active>::NextPort();
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
    listener.Accept(socket);
    listener.Close();
  }
  
  if (transferType != TransferType::List && IsFXP())
  {
    bool logging;
    
    if (transferType == TransferType::Upload ?
        !acl::AllowFxpReceive(client.User(), logging) :
        !acl::AllowFxpSend(client.User(), logging))
    {
      socket.Close();
      std::string type = transferType == TransferType::Upload ? "upload" : "download";
      if (logging)
      {
        logs::security << "User " << client.User().Name() << " attempted to fxp " << type
                       << " to " << socket.RemoteEndpoint() << logs::endl;
      }
      
      throw util::net::NetworkError("FXP " + type + " not allowed.");
    }
  }
  
  if (protection)
  {
    util::net::TLSSocket::HandshakeRole role = util::net::TLSSocket::Server;
    if ((sscnMode == ftp::SSCNMode::Client || 
         pasvType == PassiveType::CPSV) && 
        (transferType == TransferType::Upload ||
         transferType == TransferType::Download))
      role = util::net::TLSSocket::Client;  
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
    return !cfg::Get().TLSListing().Evaluate(client.User());
  }
  else
  if (IsFXP())
  {
    return !cfg::Get().TLSFxp().Evaluate(client.User());
  }

  return !cfg::Get().TLSData().Evaluate(client.User());
}

void Data::HandleControl()
{
  try
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
  catch (const util::net::NetworkError& e)
  {
    throw ftp::ControlError(std::current_exception());
  }
}

size_t Data::Read(char* buffer, size_t size)
{
  fd_set readSet;
  struct timeval tv;
  int controlSock = client.Control().socket.Socket();
  int max = std::max(socket.Socket(), controlSock);
  
  while (true)
  {
    FD_ZERO(&readSet);
    FD_SET(socket.Socket(), &readSet);
    FD_SET(controlSock, &readSet);
    memcpy(&tv, &socket.Timeout().Timeval(), sizeof(tv));
  
    int n = select(max + 1, &readSet, nullptr, nullptr, &tv);
    if (!n) throw util::net::TimeoutError();
    if (n < 0) throw util::net::NetworkSystemError(errno);
    
    if (FD_ISSET(controlSock, &readSet))
    {
      HandleControl();
    }
    
    if (FD_ISSET(socket.Socket(), &readSet))
    {
      return socket.Read(buffer, size);
    }
  }
}

void Data::Write(const char* buffer, size_t len)
{
  fd_set readSet;
  fd_set writeSet;
  struct timeval tv;
  int controlSock = client.Control().socket.Socket();
  int max = std::max(socket.Socket(), controlSock);
  
  while (true)
  {
    FD_ZERO(&readSet);
    FD_SET(controlSock, &readSet);

    FD_ZERO(&writeSet);
    FD_SET(socket.Socket(), &writeSet);

    memcpy(&tv, &socket.Timeout().Timeval(), sizeof(tv));
  
    int n = select(max + 1, &readSet, &writeSet, nullptr, &tv);
    if (!n) throw util::net::TimeoutError();
    if (n < 0) throw util::net::NetworkSystemError(errno);
    
    if (FD_ISSET(controlSock, &readSet))
    {
      HandleControl();
    }
    
    if (FD_ISSET(socket.Socket(), &writeSet))
    {
      socket.Write(buffer, len);
      if (state.Type() == TransferType::List)
        bytesWrite += len;
      return;
    }
  }
}


} /* ftp namespace */
