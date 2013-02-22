#include <cassert>
#include "db/grouputil.hpp"
#include "db/groupcache.hpp"

namespace db
{

namespace
{
std::shared_ptr<GroupCacheBase> groupCache(new GroupNoCache());
}

void SetGroupCache(const std::shared_ptr<GroupCacheBase>& cache)
{
  groupCache = cache;
}

std::string GIDToName(acl::GroupID gid)
{
  assert(groupCache);
  return groupCache->GIDToName(gid);
}

acl::GroupID NameToGID(const std::string& name)
{
  assert(groupCache);
  return groupCache->NameToGID(name);
}

} /* db namespace */
