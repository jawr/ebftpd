#include <algorithm>
#include "db/ipmasks.hpp"
#include "db/ipmaskcache.hpp"

namespace db
{

namespace
{
std::shared_ptr<IPMaskCacheBase> cache(new IPMaskNoCache());
}

void RegisterIPMaskCache(const std::shared_ptr<IPMaskCache>& newCache)
{
  cache = newCache;
}

bool IdentIPAllowed(const std::string& identAddress)
{
  return cache->IdentIPAllowed(identAddress);
}

bool IdentIPAllowed(const std::string& identAddress, acl::UserID uid)
{
  return cache->IdentIPAllowed(identAddress, uid);
}

} /* db namespace */
