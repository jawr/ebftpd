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
#include "stats/stat.hpp"

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
  acl::GroupCache::Initalize();
  acl::UserCache::Initalize();
  acl::IpMaskCache::Initalize();

  std::vector<TaskPtr> tasks;
  
  tasks.emplace_back(new db::EnsureIndex("users",
    BSON("uid" << 1 << "name" << 1)));
  tasks.emplace_back(new db::EnsureIndex("groups",
    BSON("gid" << 1 << "name" << 1)));
  tasks.emplace_back(new db::EnsureIndex("transfers", BSON("uid" << 1 << 
    "direction" << 1 << "day" << 1 << "week" << 1 << "month" << 1 << 
    "year" << 1)));
  tasks.emplace_back(new db::EnsureIndex("ipmasks",
    BSON("uid" << 1 << "mask" << 1)));

  for (auto& task: tasks)
    Pool::Queue(task);
    
  acl::UserCache::Create("biohazard", "password", "1");
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
  TaskPtr task(new db::Update("users", query, obj, true));
  Pool::Queue(task);
}

void DeleteUser(const acl::UserID& uid)
{
  mongo::Query query = QUERY("uid" << uid);
  std::vector<TaskPtr> tasks;
  tasks.emplace_back(new db::Delete("users", query));
  tasks.emplace_back(new db::Delete("ipmasks", query));
  tasks.emplace_back(new db::Delete("transfers", query));

  for (auto& task: tasks)
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

  for (auto& obj: results)
    users.push_back(bson::User::Unserialize(obj));
}

void AddIpMask(const acl::User& user, const std::string& mask)
{
  mongo::BSONObj obj = BSON("uid" << user.UID() << "mask" << mask);
  TaskPtr task(new db::Insert("ipmasks", obj));
  Pool::Queue(task);
}

void DelIpMask(const acl::User& user, const std::string& mask)
{
  mongo::Query query = QUERY("uid" << user.UID() << "mask" << mask);
  TaskPtr task(new db::Delete("ipmasks", query));
  Pool::Queue(task);  
}

void GetIpMasks(acl::UserMaskMap& userMaskMap)
{
  QueryResults results;
  mongo::Query query;
  boost::unique_future<bool> future;
  TaskPtr task(new db::Select("ipmasks", query, results, future));
  Pool::Queue(task);

  future.wait();

  if (results.size() == 0) return;

  for (auto& obj: results)
  {
    std::string mask = obj.getStringField("mask");
    acl::UserID uid = acl::UserID(obj.getIntField("uid"));
    acl::UserMaskMap::iterator it = userMaskMap.find(uid);
    if (it != userMaskMap.end())
    {
      userMaskMap[uid].emplace_back(mask);
    }
    else
    {
      std::vector<std::string> temp = {mask};
      userMaskMap.insert(std::make_pair(uid, temp));
    } 
  }
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

// stats function

void IncrementStats(const acl::User& user,
  long long kbytes, double xfertime, stats::Direction direction)
{
  std::string direction_;
  if (direction == stats::Direction::Upload)
    direction_ = "up";
  else direction_ = "dn";

  using namespace boost::posix_time;
  auto now = second_clock::local_time();
  mongo::Query query = QUERY("uid" << user.UID() << "day" << now.date().day()
    << "week" << now.date().week_number() << "month" 
    << now.date().month().as_number() << "year" << now.date().year()
    << "direction" << direction_);
  mongo::BSONObj obj = BSON(
    "$inc" << BSON("files" << 1) <<
    "$inc" << BSON("kbytes" << kbytes) <<
    "$inc" << BSON("xfertime" << xfertime));
  TaskPtr task(new db::Update("transfers", query, obj, true));
  Pool::Queue(task);
}

void DecrementStats(const acl::User& user,
  long long kbytes, double xfertime, stats::Direction direction)
{
  std::string direction_;
  if (direction == stats::Direction::Upload)
    direction_ = "up";
  else direction_ = "dn";

  using namespace boost::posix_time;
  auto now = second_clock::local_time();
  mongo::Query query = QUERY("uid" << user.UID() << "day" << now.date().day()
    << "week" << now.date().week_number() << "month" 
    << now.date().month().as_number() << "year" << now.date().year()
    << "direction" << direction_);
  mongo::BSONObj obj = BSON(
    "$inc" << BSON("files" << -1) <<
    "$inc" << BSON("kbytes" << kbytes*-1) <<
    "$inc" << BSON("xfertime" << xfertime)); // how to handle the xfertime
  TaskPtr task(new db::Update("transfers", query, obj, true));
  Pool::Queue(task);
}

// end
}
