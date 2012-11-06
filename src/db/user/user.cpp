#include "db/user/user.hpp"
#include "db/pool.hpp"
#include "db/task.hpp"
#include "db/bson/user.hpp"
#include "db/bson/userprofile.hpp"
#include "db/types.hpp"
#include "acl/groupcache.hpp"
#include "acl/group.hpp"
#include "logs/logs.hpp"

namespace db { namespace user
{

namespace
{
boost::mutex getNewUserIdMtx;
}


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

void Save(const acl::User& user)
{
  mongo::BSONObj obj = db::bson::User::Serialize(user);
  mongo::Query query = QUERY("uid" << user.UID());
  TaskPtr task(new db::Update("users", query, obj, true));
  Pool::Queue(task);
}

void Save(const acl::User& user, const std::string& field)
{
  mongo::BSONObj userObj = db::bson::User::Serialize(user);
  mongo::Query query = QUERY("uid" << user.UID());
  mongo::BSONObj obj = BSON("$set" << BSON(field << userObj[field]));
  TaskPtr task(new db::Update("users", query, obj, false));
  Pool::Queue(task);
}

void Login(acl::UserID uid)
{
  // updates login count and time
  mongo::Query query = QUERY("uid" << uid);
  mongo::BSONObj obj = BSON("$inc" << BSON("logged in" << 1) <<
    "$set" << BSON("last login" << mongo::DATENOW));
  TaskPtr task(new db::Update("userprofiles", query, obj, false));
  Pool::Queue(task);
}

void Delete(acl::UserID uid)
{
  mongo::Query query = QUERY("uid" << uid);
  std::vector<TaskPtr> tasks;
  tasks.emplace_back(new db::Delete("users", query));
  tasks.emplace_back(new db::Delete("ipmasks", query));
  tasks.emplace_back(new db::Delete("transfers", query));

  for (auto& task: tasks)
    Pool::Queue(task);      
}

boost::ptr_vector<acl::User> GetAllPtr()
{
  boost::ptr_vector<acl::User> users;

  QueryResults results;
  mongo::Query query;
  boost::unique_future<bool> future;
  TaskPtr task(new db::Select("users", query, results, future));
  Pool::Queue(task);

  future.wait();

  for (auto& obj: results)
    users.push_back(bson::User::UnserializePtr(obj));

  return users;
}

std::vector<acl::User> GetAll()
{
  std::vector<acl::User> users;

  QueryResults results;
  mongo::Query query;
  boost::unique_future<bool> future;
  TaskPtr task(new db::Select("users", query, results, future));
  Pool::Queue(task);

  future.wait();

  for (auto& obj: results)
    users.emplace_back(bson::User::Unserialize(obj));

  return users;
}


// change to objects rather than pointers
std::vector<acl::User> GetByACL(std::string acl)
{
  std::vector<acl::User> users;

  mongo::Query query;
  if (acl[0] == '-')
  {
    acl.assign(acl.begin()+1, acl.end());
    query = QUERY("name" << acl);
  }
  else if (acl[0] == '=')
  {
    acl.assign(acl.begin()+1, acl.end());
    acl::Group group;
    try
    {
      group = acl::GroupCache::Group(acl);
    }
    catch (const util::RuntimeError& e)
    {
      // group not found
      return users;
    }
    query = QUERY("$or" << BSON_ARRAY(
      BSON("primary gid" << group.GID()) << 
      BSON("secondary gids" << group.GID())
    ));
  }
  else
    query = QUERY("flags" << acl);
    
  QueryResults results;
  boost::unique_future<bool> future;
  TaskPtr task(new db::Select("users", query, results, future));
  Pool::Queue(task);

  future.wait();

  for (auto& obj: results)
    users.emplace_back(bson::User::Unserialize(obj));

  return users;
}

// end
}
}

