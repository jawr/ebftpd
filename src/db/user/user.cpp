#include "db/user/user.hpp"

namespace db
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

void GetIpMasks(acl::UserIPMaskMap& userIPMaskMap)
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

    std::vector<std::string> temp = {mask};
    std::pair<acl::UserIPMaskMap::iterator, bool> result = 
        userIPMaskMap.insert(std::make_pair(uid, temp));
    if (!result.second) result.first->second.emplace_back(mask);
  }
}

}

