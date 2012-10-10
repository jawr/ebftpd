#include "db/interface.hpp"
#include "db/task.hpp"
#include "db/pool.hpp"
#include "db/types.hpp"
#include "db/bson/user.hpp"
#include "db/bson/group.hpp"
#include "acl/types.hpp"
#include "acl/usercache.hpp"
#include "acl/groupcache.hpp"
#include "logger/logger.hpp"
#include <mongo/client/dbclient.h>
#include <boost/thread/future.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
namespace db
{
namespace
{
boost::mutex getNewUserIdMtx;
boost::mutex getNewGroupIdMtx;
}

void Initalize()
{
  db::Pool::StartThread();
  acl::UserCache::Initalize();
  acl::GroupCache::Initalize();

  std::vector<TaskPtr> tasks;
  
  tasks.emplace_back(new db::EnsureIndex("users", "uid"));
  tasks.emplace_back(new db::EnsureIndex("users", "name"));
  tasks.emplace_back(new db::EnsureIndex("groups", "gid"));
  tasks.emplace_back(new db::EnsureIndex("groups", "name"));
  for (auto task: tasks)
    Pool::Queue(task);
}


// user functions
acl::UserID GetNewUserID()
{
  boost::lock_guard<boost::mutex> lock(getNewUserIdMtx);
  
  QueryResults results;
  boost::unique_future<bool> future;
  mongo::Query query;
  query.sort("uid", 0);
  TaskPtr task(new db::Select("users", query, results, future, 1));
  Pool::Queue(task);

  future.wait();

  if (results.size() == 0) return acl::UserID(1);

  int uid = results.back().getIntField("uid");
  return acl::UserID(++uid);
}

void SaveUser(const acl::User& user)
{
  mongo::BSONObj obj = db::bson::User::Serialize(user);
  mongo::Query query = QUERY("uid" << user.UID());
  TaskPtr task(new db::Update("users", obj, query, true));
  Pool::Queue(task);
}

void GetUsers(std::vector<acl::User*>& users)
{
  QueryResults results;
  mongo::Query query;
  boost::unique_future<bool> future;
  TaskPtr task(new db::Select("users", query, results, future));
  Pool::Queue(task);

  future.wait();

  if (results.size() == 0) return;

  for (auto obj: results)
    users.push_back(bson::User::Unserialize(obj));
}


// group functions
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
  TaskPtr task(new db::Update("groups", obj, query, true));
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

  for (auto obj: results)
    groups.push_back(bson::Group::Unserialize(obj));
}

// end
}
