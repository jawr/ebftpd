#include <mongo/client/dbclient.h>
#include "db/pool.hpp"
#include "db/task.hpp"
#include "db/bson/groupprofile.hpp"
#include "db/group/groupprofile.hpp"
#include "acl/groupprofile.hpp"
#include "util/error.hpp"

namespace db
{

typedef std::shared_ptr<Task> TaskPtr;
typedef std::vector<mongo::BSONObj> QueryResults;

namespace groupprofile
{

void Delete(acl::GroupID gid)
{
  Pool::Queue(std::make_shared<db::Delete>("groupprofiles", QUERY("gid" << gid)));
}

void Save(const acl::GroupProfile& profile)
{
  mongo::BSONObj obj = db::bson::GroupProfile::Serialize(profile);
  mongo::Query query = QUERY("gid" << profile.GID());
  TaskPtr task(new db::Update("groupprofiles", query, obj, true));
  Pool::Queue(task);
}

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

std::vector<acl::GroupProfile> GetAll()
{
  std::vector<acl::GroupProfile> profiles;

  QueryResults results;
  mongo::Query query;
  boost::unique_future<bool> future;
  TaskPtr task(new db::Select("groupprofiles", query, results, future));
  Pool::Queue(task);

  future.wait();

  for (auto& obj: results)
    profiles.emplace_back(bson::GroupProfile::Unserialize(obj));

  return profiles;
}

void Set(acl::GroupID gid, mongo::BSONObj obj)
{
  obj = BSON("$set" << obj);
  mongo::Query query = QUERY("gid" << gid);
  TaskPtr task(new db::Update("groupprofiles", query, obj, false));
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

void SetMaxAllotSize(acl::GroupID gid, long long allotment)
{
  Set(gid, BSON("max allotment size" << allotment));
}

void SetMaxLogins(acl::GroupID gid, int logins) 
{
  Set(gid, BSON("max logins" << logins));
}

void SetDescription(acl::GroupID gid, const std::string& description)
{
  Set(gid, BSON("description" << description));
}

void SetComment(acl::GroupID gid, const std::string& comment)
{
  Set(gid, BSON("comment" << comment));
}

int SlotsUsed(acl::GroupID gid, const mongo::BSONObj& query)
{
  auto cmd = BSON("count" << "users" << "query" << BSON("primary gid" << gid));
  boost::unique_future<bool> future;
  mongo::BSONObj result;
  Pool::Queue(std::make_shared<db::RunCommand>(cmd, result, future));
  if (!future.get()) return -1;
  return result["n"].Number();
}

int SlotsUsed(acl::GroupID gid)
{
  return SlotsUsed(gid, mongo::BSONObj());
}

int LeechSlotsUsed(acl::GroupID gid)
{
}

int AllotSlotsUsed(acl::GroupID gid)
{
}

}
}
