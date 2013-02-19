#include <cassert>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include "db/userutil.hpp"
#include "db/usercache.hpp"
#include "db/group.hpp"
#include "acl/user.hpp"
#include "db/connection.hpp"

namespace db
{

namespace
{
std::shared_ptr<UserCacheBase> cache(new UserNoCache());
}

void SetUserCache(const std::shared_ptr<UserCacheBase>& newCache)
{
  cache = newCache;
}

std::string UIDToName(acl::UserID uid)
{
  assert(cache);
  return cache->UIDToName(uid);
}

acl::UserID NameToUID(const std::string& name)
{
  assert(cache);
  return cache->NameToUID(name);
}

acl::GroupID UIDToPrimaryGID(acl::UserID uid)
{
  assert(cache);
  return cache->UIDToPrimaryGID(uid);
}

bool IdentIPAllowed(const std::string& identAddress)
{
  assert(cache);
  return cache->IdentIPAllowed(identAddress);
}

bool IdentIPAllowed(const std::string& identAddress, acl::UserID uid)
{
  assert(cache);
  return cache->IdentIPAllowed(identAddress, uid);
}

} /* db namespace */