#include <cassert>
#include "db/grouputil.hpp"
#include "db/groupcache.hpp"

namespace db
{

namespace
{
std::shared_ptr<GroupCacheBase> cache(new GroupNoCache());
}

void SetGroupCache(const std::shared_ptr<GroupCacheBase>& newCache)
{
  cache = newCache;
}

std::string GIDToName(acl::GroupID gid)
{
  assert(cache);
  return cache->GIDToName(gid);
}

acl::GroupID NameToGID(const std::string& name)
{
  assert(cache);
  return cache->NameToGID(name);
}

} /* db namespace */
