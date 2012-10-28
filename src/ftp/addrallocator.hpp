#ifndef __FTP_ADDRALLOCATER_HPP
#define __FTP_ADDRALLOCATER_HPP

#include <cassert>
#include <vector>
#include <cstdint>
#include <boost/thread/mutex.hpp>
#include "util/net/endpoint.hpp"
#include "cfg/setting.hpp"

#include <iostream>

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
  boost::mutex mutex;
  std::vector<std::string> addresses;
  std::vector<std::string>::const_iterator it;
  
  AddrAllocatorImpl() { }
  
public:
  void SetAddrs(const std::vector<std::string>& addresses)
  {
    boost::lock_guard<boost::mutex> lock(mutex);
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
  static AddrAllocatorImpl instance;
  
public:
  static inline void SetAddrs(const std::vector<std::string>& addresses)
  { instance.SetAddrs(addresses); }

  static inline std::string NextAddr()
  { return instance.NextAddr(); }
};

template <AddrType type>
AddrAllocatorImpl AddrAllocator<type>::instance;

template class AddrAllocator<AddrType::Passive>;
template class AddrAllocator<AddrType::Active>;

} /* ftp namespace */

#endif
