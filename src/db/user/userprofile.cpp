#include "db/user/userprofile.hpp"

namespace db { namespace userprofile
{

acl::UserProfile* Get(const acl::UserID& uid)
{
  QueryResults results;
  mongo::Query query = QUERY("uid" << uid);
  boost::unique_future<bool> future;
  TaskPtr task(new db::Select("userprofiles", query, results, future));
  Pool::Queue(task);

  future.wait();

  if (results.size() == 0) 
    throw util::RuntimeError("Unable to get UserProfile.");

  for (auto& obj: results)
    return bson::UserProfile::Unserialize(obj);
  return nullptr; // -Wreturn-type will never get here
}

void Save(const acl::UserProfile& profile)
{
  mongo::BSONObj obj = db::bson::UserProfile::Serialize(profile);
  mongo::Query query = QUERY("uid" << profile.UID());
  TaskPtr task(new db::Update("userprofiles", query, obj, true));
  Pool::Queue(task);
}

void GetAll(std::vector<acl::UserProfile*>& profiles)
{
  QueryResults results;
  mongo::Query query;
  boost::unique_future<bool> future;
  TaskPtr task(new db::Select("userprofiles", query, results, future));
  Pool::Queue(task);

  future.wait();

  if (results.size() == 0) return;

  for (auto& obj: results)
    profiles.push_back(bson::UserProfile::Unserialize(obj));
}

// end
}
}
