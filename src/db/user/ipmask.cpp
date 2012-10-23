#include "db/user/ipmask.hpp"
#include "db/task.hpp"
#include "db/types.hpp"
#include "db/pool.hpp"

namespace db { namespace ipmask
{

void Add(const acl::User& user, const std::string& mask)
{
  mongo::BSONObj obj = BSON("uid" << user.UID() << "mask" << mask);
  TaskPtr task(new db::Insert("ipmasks", obj));
  Pool::Queue(task);
}

void Delete(const acl::User& user, const std::string& mask)
{
  mongo::Query query = QUERY("uid" << user.UID() << "mask" << mask);
  TaskPtr task(new db::Delete("ipmasks", query));
  Pool::Queue(task);  
}

void GetAll(acl::UserIPMaskMap& userIPMaskMap)
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

// end
}
}
