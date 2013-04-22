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

#ifndef __FTP_ADDRALLOCATER_HPP
#define __FTP_ADDRALLOCATER_HPP

#include <memory>
#include <cassert>
#include <vector>
#include <cstdint>
#include <mutex>
#include <boost/thread/once.hpp>
#include "util/net/endpoint.hpp"
#include "cfg/get.hpp"

namespace ftp
{

enum class AddrType
{
  Active,
  Passive
};

template <AddrType type>
class AddrAllocator;

class AddrAllocatorImpl
{
  std::mutex mutex;
  std::vector<std::string> addresses;
  std::vector<std::string>::const_iterator it;
  
  AddrAllocatorImpl() = default;
  
public:
  void SetAddrs(const std::vector<std::string>& addresses)
  {
    std::lock_guard<std::mutex> lock(mutex);
    this->addresses = addresses;
    it  = this->addresses.begin();
  }

  std::string inline NextAddr()
  {
    if (addresses.empty()) return "";
    while (true)
    {
      if (it == addresses.end()) it = addresses.begin();
      return *it++;
    }
  }
  
  friend class AddrAllocator<AddrType::Active>;
  friend class AddrAllocator<AddrType::Passive>;
};

template <AddrType type>
class AddrAllocator
{
  static std::unique_ptr<AddrAllocatorImpl> instance;
  static boost::once_flag instanceOnce;
  
  static void CreateInstance() { instance.reset(new AddrAllocatorImpl()); }
  
public:
  static AddrAllocatorImpl& Get()
  {
    boost::call_once(&CreateInstance, instanceOnce);
    return *instance;
  }
};

template <AddrType type>
std::unique_ptr<AddrAllocatorImpl> AddrAllocator<type>::instance;
template <AddrType type>
boost::once_flag AddrAllocator<type>::instanceOnce = BOOST_ONCE_INIT;

template class AddrAllocator<AddrType::Passive>;
template class AddrAllocator<AddrType::Active>;

inline void InitialiseAddrAllocators()
{
  cfg::ConnectUpdatedSlot([]() { AddrAllocator<ftp::AddrType::Active>::Get().SetAddrs(cfg::Get().ActiveAddr()); });
  cfg::ConnectUpdatedSlot([]() { AddrAllocator<ftp::AddrType::Passive>::Get().SetAddrs(cfg::Get().PasvAddr()); });
}

} /* ftp namespace */

#endif
