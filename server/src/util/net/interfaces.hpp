#ifndef __UTIL_NET_INTERFACES_HPP
#define __UTIL_NET_INTERFACES_HPP

#include <vector>
#include <string>
#include <net/if.h>
#include <map>
#include "util/net/ipaddress.hpp"

namespace util { namespace net
{

class Interfaces;

class Interface
{
  std::string name;
  std::vector<IPAddress> addresses;
  
  Interface(const std::string& name) : name(name) { }

public:
  typedef std::vector<IPAddress>::const_iterator const_iterator;
  typedef std::vector<IPAddress>::size_type size_type;

  const std::string& Name();
  const std::vector<IPAddress>& Addresses() const { return addresses; }
  
  size_type size() const { return addresses.size(); }
  const_iterator begin() const { return addresses.begin(); }
  const_iterator end() const { return addresses.end(); }
  
  friend class Interfaces;
};

class Interfaces
{
  std::map<std::string, Interface> interfaces;
  
  template<typename T1, typename T2>
  T2& TakeSecond(const std::pair<T1, T2> &pair) 
  {
    return pair.second;
  }
  
public:
  typedef std::map<std::string, Interface>::const_iterator const_iterator;
  typedef std::map<std::string, Interface>::size_type size_type;
  
  Interfaces(bool deferLoad = false)
  {
    if (!deferLoad) Load();
  }
  
  void Load();
  
  size_type size() const { return interfaces.size(); }
  bool empty() const { return interfaces.empty(); }
  const_iterator begin() const { return interfaces.begin();  }
  const_iterator end() const { return interfaces.end(); }
};

bool FindPartnerIP(const IPAddress& ip, IPAddress& partner);

} /* net namespace */
} /* util namespace */

#endif
