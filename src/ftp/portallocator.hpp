#ifndef __FTP_PORTALLOCATER_HPP
#define __FTP_PORTALLOCATER_HPP

#include <cassert>
#include <vector>
#include <cstdint>
#include <boost/thread/mutex.hpp>
#include "util/net/endpoint.hpp"
#include "cfg/setting.hpp"

#include <iostream>

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
  boost::mutex mutex;
  cfg::setting::Ports ports;
  std::vector<cfg::setting::PortRange>::const_iterator it;
  uint16_t nextPort;
  
  PortAllocatorImpl() : nextPort(0) {}
  
public:
  void SetPorts(const cfg::setting::Ports& ports)
  {
    boost::lock_guard<boost::mutex> lock(mutex);
    this->ports = ports;
    it = this->ports.Ranges().begin();
  }

  uint16_t inline NextPort()
  {
    boost::lock_guard<boost::mutex> lock(mutex);
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

} /* ftp namespace */

#endif
