#ifndef __FTP_PORTALLOCATER_HPP
#define __FTP_PORTALLOCATER_HPP

#include <cassert>
#include <vector>
#include <cstdint>
#include <mutex>
#include "util/net/endpoint.hpp"
#include "cfg/get.hpp"

namespace ftp
{

enum class PortType
{
  Active,
  Passive
};

template <PortType type>
class PortAllocator;

class PortAllocatorImpl
{
  std::mutex mutex;
  cfg::setting::Ports ports;
  std::vector<cfg::setting::PortRange>::const_iterator it;
  uint16_t nextPort;
  
  PortAllocatorImpl() : nextPort(0) { }
  
public:
  void SetPorts(const cfg::setting::Ports& ports)
  {
    std::lock_guard<std::mutex> lock(mutex);
    this->ports = ports;
    it = this->ports.Ranges().begin();
  }

  uint16_t inline NextPort()
  {
    std::lock_guard<std::mutex> lock(mutex);
    if (ports.Ranges().empty()) return util::net::Endpoint::AnyPort();
    if (!nextPort) 
    {
      nextPort = it->From();
      return nextPort++;
    }
    
    while (true)
    {
      if (nextPort <= it->To()) return nextPort++;
      if (++it == ports.Ranges().end())
        it = ports.Ranges().begin();
      nextPort = it->From();
    }
  }
  
  friend class PortAllocator<PortType::Active>;
  friend class PortAllocator<PortType::Passive>;
};

template <PortType type>
class PortAllocator
{
  static PortAllocatorImpl instance;
  
public:
  static inline void SetPorts(const cfg::setting::Ports& ports)
  { instance.SetPorts(ports); }

  static inline uint16_t NextPort()
  { return instance.NextPort(); }
};

template <PortType type>
PortAllocatorImpl PortAllocator<type>::instance;

template class PortAllocator<PortType::Passive>;
template class PortAllocator<PortType::Active>;

inline void InitialisePortAllocators()
{
  cfg::ConnectUpdatedSlot([]() { PortAllocator<ftp::PortType::Active>::SetPorts(cfg::Get().ActivePorts()); });
  cfg::ConnectUpdatedSlot([]() { PortAllocator<ftp::PortType::Passive>::SetPorts(cfg::Get().PasvPorts()); });
}

} /* ftp namespace */

#endif
