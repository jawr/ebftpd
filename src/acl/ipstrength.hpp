#ifndef __ACL_IPSTRENGTH_HPP
#define __ACL_IPSTRENGTH_HPP

#include <string>
#include <cctype>
#include "util/error.hpp"

namespace acl
{

class IPStrength
{
  unsigned numOctets;
  bool isHostname;
  bool hasIdent;
  
public:
  explicit IPStrength() : numOctets(2), isHostname(false), hasIdent(true) { }
  explicit IPStrength(unsigned numOctets, bool isHostname, bool hasIdent) : 
    numOctets(numOctets), isHostname(isHostname), hasIdent(hasIdent) { }
  explicit IPStrength(std::string ip) : 
    numOctets(0), isHostname(false), hasIdent(true)
  {
    std::string wildcards("*?[]");
    
    std::string::size_type pos = ip.find_last_of('@');
    if (pos != std::string::npos)
    {
      std::string ident = ip.substr(0, pos);
      if (ident.find_first_of(wildcards) != std::string::npos)
        hasIdent = false;
      ip.erase(0, pos + 1);
    }
    else
      hasIdent = false;
      
    auto it = ip.begin();
    for (; it != ip.end(); ++it)
    {
      if (*it == '.' || *it == ':') ++numOctets;
      else
      if (!std::isdigit(*it))
      {
        if (wildcards.find_first_of(*it) != std::string::npos) break;
        if (std::isalpha(*it) || *it == '-')
        {
          isHostname = true;
          break;
        }
      }
    }
    
    if (it == ip.end()) ++numOctets;
  }
  
  util::Error Allowed(const IPStrength& userStrength) const
  {
    if (userStrength.isHostname)
    {
      if (!isHostname)
        return util::Error::Failure("Hostname's not allowed");
    }
    else
    {
      if (userStrength.numOctets < numOctets)
        return util::Error::Failure("Not enough octets specified");
    }
    
    if (!userStrength.hasIdent && hasIdent)
      return util::Error::Failure("No ident specified");
      
    return util::Error::Success();
  }
  
  unsigned NumOctets() const { return numOctets; }
  bool IsHostname() const { return isHostname; }
  bool HasIdent() const { return hasIdent; }
};

} /* acl namespace */

#endif
