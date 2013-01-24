#include <cassert>
#include "acl/util.hpp"
#include "util/error.hpp"
#include "acl/groupcache.hpp"
#include "acl/usercache.hpp"
#include "acl/ipmaskcache.hpp"
#include "util/verify.hpp"

namespace acl
{

void CreateDefaults()
{
  if (acl::GroupCache::Create("ebftpd"))
    assert(acl::GroupCache::NameToGID("ebftpd") == 0);
  
  if (!acl::GroupCache::Exists(0))
  {
    throw util::RuntimeError("Root group (GID 0) doesn't exist.");
  }

  if (acl::UserCache::Create("ebftpd", "ebftpd", "1", 0))
  {
    assert(acl::UserCache::NameToUID("ebftpd") == 0);
    acl::UserCache::SetPrimaryGID("ebftpd", 0);
    acl::IpMaskCache::Add(0, "*@127.0.0.1");
  }
  
  if (!acl::UserCache::Exists(0))
  {
    throw util::RuntimeError("Root user (UID 0) doesn't exist.");
  }
  
  if (acl::UserCache::Create("biohazard", "password", "1", 0))
  {
    verify(acl::IpMaskCache::Add(acl::UserCache::NameToUID("biohazard"), "*@127.0.0.1"));
  }
  
  if (acl::UserCache::Create("io", "password", "1", 0))
  {
    verify(acl::IpMaskCache::Add(acl::UserCache::NameToUID("io"), "*@127.0.0.1"));
  }
}

} /* acl namespace */