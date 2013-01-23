#include "ftp/portallocator.hpp"
#include "ftp/addrallocator.hpp"
#include "util/net/interfaces.hpp"
#include "ftp/data.hpp"
#include "logs/logs.hpp"
#include "acl/allowfxp.hpp"
#include "acl/user.hpp"
#include "ftp/client.hpp"
#include "cfg/get.hpp"

namespace ftp
{

template <> const char* util::EnumStrings<EPSVMode>::values[]
{
  "EXTENDED",
  "NORMAL"
};

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
    if (!acl::AllowFxp(transferType, client.User(), logging))
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
  
    if (role == util::net::TLSSocket::Client) logs::debug << "TLS Client handshake" << logs::endl;
    else logs::debug << "TLS Server handshake" << logs::endl;
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

} /* ftp namespace */
