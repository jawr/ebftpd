#include "db/pool.hpp"
#include "db/task.hpp"
#include "db/types.hpp"
#include "db/bson/groupprofile.hpp"
#include "db/group/groupprofile.hpp"

namespace db { namespace groupprofile
{

acl::GroupProfile Get(acl::GroupID gid)
{
  QueryResults results;
  mongo::Query query = QUERY("gid" << gid);
  boost::unique_future<bool> future;
  TaskPtr task(new db::Select("groupprofiles", query, results, future));
  Pool::Queue(task);

  future.wait();

  if (results.empty())
    throw util::RuntimeError("Unable to get GroupProfile");

  return bson::GroupProfile::Unserialize(*results.begin());
}

void Set(acl::GroupID gid, mongo::BSONObj obj)
{
  obj = BSON("$set" << obj);
  mongo::Query query = QUERY("gid" << gid);
  TaskPtr task(new db::Update("userprofiles", query, obj, false));
  Pool::Queue(task);
}

void SetSlots(acl::GroupID gid, int slots)
{
  Set(gid, BSON("slots" << slots));
}

void SetLeechSlots(acl::GroupID gid, int slots)
{
  Set(gid, BSON("leech slots" << slots));
}

void SetAllotSlots(acl::GroupID gid, int slots)
{
  Set(gid, BSON("allotment slots" << slots));
}

void SetMaxAllotSlots(acl::GroupID gid, int slots)
{
  Set(gid, BSON("max allotment slots" << slots));
}

void SetMaxLogins(acl::GroupID gid, int maxLogins) 
{
  Set(gid, BSON("max logins" << maxLogins));
}

void SetDescription(acl::GroupID gid, const std::string& description)
{
  Set(gid, BSON("description" << description));
}

void SetComment(acl::GroupID gid, const std::string& comment)
{
  Set(gid, BSON("comment" << comment));
}
// end
}
}
