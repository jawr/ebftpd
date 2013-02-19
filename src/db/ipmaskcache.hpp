#ifndef __DB_IPMASKCACHE_HPP
#define __DB_IPMASKCACHE_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <boost/thread/mutex.hpp>
#include "db/replicable.hpp"
#include "acl/types.hpp"

namespace db
{

struct IPMaskCacheBase
{
  virtual ~IPMaskCacheBase() { }
  virtual bool IdentIPAllowed(const std::string& identAddress) = 0;  
  virtual bool IdentIPAllowed(const std::string& identAddress, acl::UserID uid) = 0;  
};

class IPMaskCache : 
  public IPMaskCacheBase,
  public Replicable
{
  boost::mutex mutex;
  std::unordered_map<acl::UserID, std::vector<std::string>> ipMasks;

public:
  bool IdentIPAllowed(const std::string& identAddress);
  bool IdentIPAllowed(const std::string& identAddress, acl::UserID uid);
  
  bool Replicate() { return false; }
};

class IPMaskNoCache : public IPMaskCacheBase
{
  std::vector<std::string> Lookup(acl::UserID = -1);
  
public:
  bool IdentIPAllowed(const std::string& identAddress);
  bool IdentIPAllowed(const std::string& identAddress, acl::UserID uid);
};

} /* db namespace */

#endif
