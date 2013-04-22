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

#ifndef __FTP_PORTALLOCATER_HPP
#define __FTP_PORTALLOCATER_HPP

#include <memory>
#include <cassert>
#include <vector>
#include <mutex>
#include <boost/thread/once.hpp>
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
  cfg::Ports ports;
  std::vector<cfg::PortRange>::const_iterator it;
  int nextPort;
  
  PortAllocatorImpl() : nextPort(0) { }
  
public:
  void SetPorts(const cfg::Ports& ports)
  {
    std::lock_guard<std::mutex> lock(mutex);
    this->ports = ports;
    it = this->ports.Ranges().begin();
  }

  int inline NextPort()
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
  static std::unique_ptr<PortAllocatorImpl> instance;
  static boost::once_flag instanceOnce;
  
  static void CreateInstance() { instance.reset(new PortAllocatorImpl()); }
  
public:
  static PortAllocatorImpl& Get()
  {
    boost::call_once(&CreateInstance, instanceOnce);
    return *instance;
  }
};

template <PortType type>
std::unique_ptr<PortAllocatorImpl> PortAllocator<type>::instance;
template <PortType type>
boost::once_flag PortAllocator<type>::instanceOnce = BOOST_ONCE_INIT;

template class PortAllocator<PortType::Passive>;
template class PortAllocator<PortType::Active>;

inline void InitialisePortAllocators()
{
  cfg::ConnectUpdatedSlot([]() { PortAllocator<ftp::PortType::Active>::Get().SetPorts(cfg::Get().ActivePorts()); });
  cfg::ConnectUpdatedSlot([]() { PortAllocator<ftp::PortType::Passive>::Get().SetPorts(cfg::Get().PasvPorts()); });
}

} /* ftp namespace */

#endif
