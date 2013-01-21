#include <ctime>
#include <mongo/client/dbclient.h>
#include <boost/thread/future.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "db/interface.hpp"
#include "db/task.hpp"
#include "db/pool.hpp"
#include "db/types.hpp"
#include "db/bson/user.hpp"
#include "db/bson/group.hpp"
#include "acl/types.hpp"
#include "acl/usercache.hpp"
#include "acl/groupcache.hpp"
#include "acl/ipmaskcache.hpp"
#include "acl/types.hpp"
#include "logs/logs.hpp"
#include "db/error.hpp"
#include "util/verify.hpp"

namespace db
{

void Initialize()
{
  db::Pool::StartThread();
  acl::GroupCache::Initialize();
  acl::UserCache::Initialize();
  acl::IpMaskCache::Initialize();

  Pool::Queue(TaskPtr(new db::EnsureIndex("users",
    BSON("uid" << 1 << "name" << 1))));
  Pool::Queue(TaskPtr(new db::EnsureIndex("groups",
    BSON("gid" << 1 << "name" << 1))));
  Pool::Queue(TaskPtr(new db::EnsureIndex("transfers", BSON("uid" << 1 << 
    "direction" << 1 << "day" << 1 << "week" << 1 << "month" << 1 << 
    "year" << 1))));
  Pool::Queue(TaskPtr(new db::EnsureIndex("ipmasks",
    BSON("uid" << 1 << "mask" << 1))));

  if (acl::GroupCache::Create("ebftpd"))
    assert(acl::GroupCache::NameToGID("ebftpd") == 0);
  
  if (!acl::GroupCache::Exists(0))
  {
    throw db::DBError("Root group (GID 0) doesn't exist.");
  }

  if (acl::UserCache::Create("ebftpd", "ebftpd", "1", 0))
  {
    assert(acl::UserCache::NameToUID("ebftpd") == 0);
    acl::UserCache::SetPrimaryGID("ebftpd", 0);
    acl::IpMaskCache::Add(0, "*@127.0.0.1");
  }
  
  if (!acl::UserCache::Exists(0))
  {
    throw db::DBError("Root user (UID 0) doesn't exist.");
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

void Cleanup()
{
  Pool::StopThread();
}

// end
}
