#include "db/group/group.hpp"
#include "db/types.hpp"
#include "db/pool.hpp"
#include "db/task.hpp"
#include "db/bson/group.hpp"
#include "acl/group.hpp"
#include "acl/types.hpp"
#include "logs/logs.hpp"
#include "db/exception.hpp"

namespace db { namespace group
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

  try
  {
    return results.back().getIntField("gid");
  }
  catch (const mongo::DBException& e)
  {
    logs::db << "Error while getting new group id: " << e.what() << logs::endl;
  }
  throw db::DBError("Unable to get new group id.");
}

void Save(const acl::Group& group)
{
  mongo::BSONObj obj = db::bson::Group::Serialize(group);
  mongo::Query query = QUERY("gid" << group.GID());
  TaskPtr task(new db::Update("groups", query, obj, true));
  Pool::Queue(task);
}

void GetAll(boost::ptr_vector<acl::Group>& groups)
{
  groups.clear();

  QueryResults results;
  mongo::Query query;
  boost::unique_future<bool> future;
  TaskPtr task(new db::Select("groups", query, results, future));
  Pool::Queue(task);

  future.wait();

  for (auto& obj: results)
    groups.push_back(bson::Group::Unserialize(obj));
}

void Delete(acl::GroupID gid)
{
  mongo::Query query = QUERY("gid" << gid);
  std::vector<TaskPtr> tasks;
  tasks.emplace_back(new db::Delete("groups", query));
  // add groupprofile here later

  for (auto& task: tasks)
    Pool::Queue(task);
}

}
}

