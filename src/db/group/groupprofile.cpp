#include "db/pool.hpp"
#include "db/task.hpp"
#include "db/types.hpp"
#include "db/bson/groupprofile.hpp"
#include "db/group/groupprofile.hpp"

namespace db { namespace groupprofile
{

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

void Set(acl::GroupID gid, mongo::BSONObj obj)
{
  obj = BSON("$set" << obj);
  mongo::Query query = QUERY("gid" << gid);
  TaskPtr task(new db::Update("userprofiles", query, obj, false));
  Pool::Queue(task);
}

util::Error SetSlots(acl::GroupID gid, const std::string& slots)
{
  int i;
  try
  {
    i = boost::lexical_cast<int>(slots);
  }
  catch (const boost::bad_lexical_cast& e)
  {
    return util::Error::Failure("Failed to parse number!");
  }
  Set(gid, BSON("slots" << i));
  return util::Error::Success();
}

util::Error SetLeechSlots(acl::GroupID gid, const std::string& slots)
{
  int i;
  try
  {
    i = boost::lexical_cast<int>(slots);
  }
  catch (const boost::bad_lexical_cast& e)
  {
    return util::Error::Failure("Failed to parse number!");
  }
  Set(gid, BSON("leech slots" << i));
  return util::Error::Success();
}

util::Error SetAllotSlots(acl::GroupID gid, const std::string& slots)
{
  int i;
  try
  {
    i = boost::lexical_cast<int>(slots);
  }
  catch (const boost::bad_lexical_cast& e)
  {
    return util::Error::Failure("Failed to parse number!");
  }
  Set(gid, BSON("allotment slots" << i));
  return util::Error::Success();
}

util::Error SetMaxAllotSlots(acl::GroupID gid, const std::string& slots)
{
  int i;
  try
  {
    i = boost::lexical_cast<int>(slots);
  }
  catch (const boost::bad_lexical_cast& e)
  {
    return util::Error::Failure("Failed to parse number!");
  }
  Set(gid, BSON("max allotment slots" << i));
  return util::Error::Success();
}

util::Error SetMaxLogins(acl::GroupID gid, int maxLogins) 
{
  int i;
  try
  {
    i = boost::lexical_cast<int>(maxLogins);
  }
  catch (const boost::bad_lexical_cast& e)
  {
    return util::Error::Failure("Failed to parse number!");
  }
  Set(gid, BSON("max logins" << i));
  return util::Error::Success();
}

util::Error SetDescription(acl::GroupID gid, const std::string& description)
{
  Set(gid, BSON("description" << description));
  return util::Error::Success();
}

util::Error SetComment(acl::GroupID gid, const std::string& comment)
{
  Set(gid, BSON("comment" << comment));
  return util::Error::Success();
}
// end
}
}
