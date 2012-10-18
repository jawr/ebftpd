#include "db/group/group.hpp"
#include "db/types.hpp"
#include "db/pool.hpp"
#include "db/task.hpp"
#include "db/bson/group.hpp"
#include "acl/group.hpp"
#include "acl/types.hpp"

namespace db
{

namespace
{
boost::mutex getNewGroupIdMtx;
}

acl::GroupID GetNewGroupID()
{
  boost::lock_guard<boost::mutex> lock(getNewGroupIdMtx);
  
  QueryResults results;
  boost::unique_future<bool> future;
  mongo::Query query;
  query.sort("gid", 0);
  TaskPtr task(new db::Select("groups", query, results, future, 1));
  Pool::Queue(task);

  future.wait();

  if (results.size() == 0) return acl::GroupID(1);

  int gid = results.back().getIntField("gid");
  return acl::GroupID(++gid);
}

void SaveGroup(const acl::Group& group)
{
  mongo::BSONObj obj = db::bson::Group::Serialize(group);
  mongo::Query query = QUERY("gid" << group.GID());
  TaskPtr task(new db::Update("groups", query, obj, true));
  Pool::Queue(task);
}

void GetGroups(std::vector<acl::Group*>& groups)
{
  QueryResults results;
  mongo::Query query;
  boost::unique_future<bool> future;
  TaskPtr task(new db::Select("groups", query, results, future));
  Pool::Queue(task);

  future.wait();

  if (results.size() == 0) return;

  for (auto& obj: results)
    groups.push_back(bson::Group::Unserialize(obj));
}

}

