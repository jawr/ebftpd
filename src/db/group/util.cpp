#include <cassert>
#include "db/group/util.hpp"
#include "db/group/groupcache.hpp"
#include "db/group/serialization.hpp"
#include "db/connection.hpp"

namespace db
{

namespace
{

struct GroupNoCache : public GroupCacheBase
{
  std::string GIDToName(acl::GroupID gid);
  acl::GroupID NameToGID(const std::string& name);
};

std::string GroupNoCache::GIDToName(acl::GroupID gid)
{
  NoErrorConnection conn;  
  auto fields = BSON("gid" << 1 << "name" << 1 << "primary gid" << 1);
  auto data = conn.QueryOne<GroupPair>("groups", QUERY("gid" << gid), &fields);
  if (!data) return "unknown";
  return data->name;
}

acl::GroupID GroupNoCache::NameToGID(const std::string& name)
{
  NoErrorConnection conn;  
  auto fields = BSON("gid" << 1 << "name" << 1 << "primary gid" << 1);
  auto data = conn.QueryOne<GroupPair>("groups", QUERY("name" << name), &fields);
  if (!data) return -1;
  return data->gid;
}

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
