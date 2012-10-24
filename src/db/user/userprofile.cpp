#include <boost/lexical_cast.hpp>
#include "db/user/userprofile.hpp"
#include "db/task.hpp"
#include "db/types.hpp"
#include "db/pool.hpp"
#include "db/bson/userprofile.hpp"

namespace db { namespace userprofile
{

acl::UserProfile Get(const acl::UserID& uid)
{
  QueryResults results;
  mongo::Query query = QUERY("uid" << uid);
  boost::unique_future<bool> future;
  TaskPtr task(new db::Select("userprofiles", query, results, future));
  Pool::Queue(task);

  future.wait();

  if (results.size() == 0) 
    throw util::RuntimeError("Unable to get UserProfile.");

  return bson::UserProfile::Unserialize(*results.begin());
}

void Save(const acl::UserProfile& profile)
{
  mongo::BSONObj obj = db::bson::UserProfile::Serialize(profile);
  mongo::Query query = QUERY("uid" << profile.UID());
  TaskPtr task(new db::Update("userprofiles", query, obj, true));
  Pool::Queue(task);
}

void GetAll(std::vector<acl::UserProfile>& profiles)
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

void Set(const acl::UserID uid, mongo::BSONObj obj)
{
  obj = BSON("$set" << obj);
  mongo::Query query = QUERY("uid" << uid);
  TaskPtr task(new db::Update("userprofiles", query, obj, false));
  Pool::Queue(task);
}

util::Error SetRatio(const acl::UserID uid, const std::string& value)
{
  int i;
  try
  {
    i = boost::lexical_cast<int>(value);
  }
  catch (const boost::bad_lexical_cast& e)
  {
    return util::Error::Failure("Failed to parse number!");
  }
  Set(uid, BSON("ratio" << i));
  return util::Error::Success();
}

util::Error SetWeeklyAllotment(const acl::UserID uid, const std::string& value)
{
  int i;
  try
  {
    i = boost::lexical_cast<int>(value);
  }
  catch (const boost::bad_lexical_cast& e)
  {
    return util::Error::Failure("Failed to parse number!");
  }
  Set(uid, BSON("weekly allotment" << i));
  return util::Error::Success();
}
  
util::Error SetHomeDir(const acl::UserID uid, const std::string& value)
{
  Set(uid, BSON("home dir" << value));
  return util::Error::Success();
}

util::Error SetStartupDir(const acl::UserID uid, const std::string& value)
{
  Set(uid, BSON("startup dir" << value));
  return util::Error::Success();
}

util::Error SetIdleTime(const acl::UserID uid, const std::string& value)
{
  int i;
  try
  {
    i = boost::lexical_cast<int>(value);
  }
  catch (const boost::bad_lexical_cast& e)
  {
    return util::Error::Failure("Failed to parse number!");
  }
  Set(uid, BSON("idle time" << i));
  return util::Error::Success();
}

util::Error SetExpires(const acl::UserID uid, const std::string& value)
{
  Set(uid, BSON("expires" << value));
  return util::Error::Success();
}

util::Error SetNumLogins(const acl::UserID uid, const std::string& value)
{
  int i;
  try
  {
    i = boost::lexical_cast<int>(value);
  }
  catch (const boost::bad_lexical_cast& e)
  {
    return util::Error::Failure("Failed to parse number!");
  }
  Set(uid, BSON("idle time" << i));
  return util::Error::Success();
}

util::Error SetTagline(const acl::UserID uid, const std::string& value)
{
  Set(uid, BSON("tagline" << value));
  return util::Error::Success();
}

util::Error SetComment(const acl::UserID uid, const std::string& value)
{
  Set(uid, BSON("comment" << value));
  return util::Error::Success();
}

util::Error SetMaxDlSpeed(const acl::UserID uid, const std::string& value)
{
  int i;
  try
  {
    i = boost::lexical_cast<int>(value);
  }
  catch (const boost::bad_lexical_cast& e)
  {
    return util::Error::Failure("Failed to parse number!");
  }
  Set(uid, BSON("max dl speed" << i));
  return util::Error::Success();
}

util::Error SetMaxUlSpeed(const acl::UserID uid, const std::string& value)
{
  int i;
  try
  {
    i = boost::lexical_cast<int>(value);
  }
  catch (const boost::bad_lexical_cast& e)
  {
    return util::Error::Failure("Failed to parse number!");
  }
  Set(uid, BSON("max ul speed" << i));
  return util::Error::Success();
}

util::Error SetMaxSimDl(const acl::UserID uid, const std::string& value)
{
  int i;
  try
  {
    i = boost::lexical_cast<int>(value);
  }
  catch (const boost::bad_lexical_cast& e)
  {
    return util::Error::Failure("Failed to parse number!");
  }
  Set(uid, BSON("max sim dl" << i));
  return util::Error::Success();
}

util::Error SetMaxSimUl(const acl::UserID uid, const std::string& value)
{
  int i;
  try
  {
    i = boost::lexical_cast<int>(value);
  }
  catch (const boost::bad_lexical_cast& e)
  {
    return util::Error::Failure("Failed to parse number!");
  }
  Set(uid, BSON("max sim ul" << i));
  return util::Error::Success();
}


// end
}
}
