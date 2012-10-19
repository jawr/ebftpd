#include "ftp/portallocator.hpp"
#include "ftp/addrallocator.hpp"
#include "util/net/interfaces.hpp"
#include "ftp/data.hpp"
#include "logs/logs.hpp"
#include "acl/allowfxp.hpp"
#include "acl/user.hpp"
#include "ftp/client.hpp"

namespace ftp
{

void Data::InitPassive(util::net::Endpoint& ep, PassiveType   pasvType)
{
  using namespace util::net;

  passiveMode = true;
  socket.Close();
  listener.Close();
  
  boost::optional<util::net::IPAddress> ip;
  // unable to use alternative pasv_addr if espv mode isn't Full
  // should we fail -- or substitute the ip 
  // from the client.Control.LocalEndpoint like now?
  if (pasvType != PassiveType::EPSV || epsvMode == ::ftp::EPSVMode::Full)
  {
    std::string firstAddr;
    while (true)
    {
      std::string addr = AddrAllocator<AddrType::Passive>::NextAddr();
      if (addr.empty()) break;
      
      if (addr == firstAddr)
        throw NetworkError("Unable to find a valid local address");
      
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
      throw util::net::NetworkError("All ports exhausted");
      
    try
    {
      ep = Endpoint(*ip, port);
      listener.Listen(ep);
      break;
    }
    catch (const util::net::NetworkSystemError& e)
    {
      if (e.Errno() != EADDRINUSE)
        throw;
    }
  }
}

void Data::InitActive(const util::net::Endpoint& ep)
{
  passiveMode = false;
  socket.Close();
  listener.Close();
  
  boost::optional<util::net::IPAddress> localIP;
  std::string firstAddr;
  while (true)
  {
    std::string addr = AddrAllocator<AddrType::Active>::NextAddr();
    if (addr.empty()) break;
    
    if (addr == firstAddr)
      throw util::net::NetworkError("Unable to find a valid local address");
    
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
      throw util::net::NetworkError("All ports exhausted");
      
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
  if (passiveMode)
  {
    listener.Accept(socket);
    listener.Close();
  }
  
  if (transferType != TransferType::List &&
      socket.RemoteEndpoint().IP() != client.Control().RemoteEndpoint().IP())
  {
    bool logging;
    if (!acl::AllowFxp(transferType, client.User(), logging))
    {
      socket.Close();
      std::string type = transferType == TransferType::Upload ?
                         "upload" : "download";
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
    socket.HandshakeTLS(util::net::TLSSocket::Server);
  }
}

} /* ftp namespace */
