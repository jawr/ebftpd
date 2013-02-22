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
std::shared_ptr<UserCacheBase> userCache(new UserNoCache());
}

void SetUserCache(const std::shared_ptr<UserCacheBase>& cache)
{
  userCache = cache;
}

std::string UIDToName(acl::UserID uid)
{
  assert(userCache);
  return userCache->UIDToName(uid);
}

acl::UserID NameToUID(const std::string& name)
{
  assert(userCache);
  return userCache->NameToUID(name);
}

acl::GroupID UIDToPrimaryGID(acl::UserID uid)
{
  assert(userCache);
  return userCache->UIDToPrimaryGID(uid);
}

bool IdentIPAllowed(const std::string& identAddress)
{
  assert(userCache);
  return userCache->IdentIPAllowed(identAddress);
}

bool IdentIPAllowed(const std::string& identAddress, acl::UserID uid)
{
  assert(userCache);
  return userCache->IdentIPAllowed(identAddress, uid);
}

} /* db namespace */