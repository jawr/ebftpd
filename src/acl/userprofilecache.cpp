#include <boost/lexical_cast.hpp>
#include "acl/userprofilecache.hpp"
#include "acl/userprofile.hpp"
#include "util/error.hpp"
#include "db/user/user.hpp"

namespace acl
{

UserProfileCache UserProfileCache::instance;

UserProfileCache::~UserProfileCache()
{
  while (!byUID.empty())
  {
    delete byUID.begin()->second;
    byUID.erase(byUID.begin());
  }
}

void UserProfileCache::Save(const acl::UserID& uid)
{
  db::SaveUserProfile(*instance.byUID[uid]);
}

util::Error UserProfileCache::Ensure(UserID uid)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByUIDMap::iterator it = instance.byUID.find(uid);
  if (it != instance.byUID.end())
    util::Error::Success();
  acl::UserProfile* profile;
  try
  {
    profile = new acl::UserProfile(uid);
    instance.byUID.insert(std::make_pair(uid, profile));
  }
  catch (const util::RuntimeError& e)
  {
    logs::error << "Failed to insert UID to UserProfileCache: " <<
      e.Message() << logs::endl;
    delete profile;
    return util::Error::Failure(e.Message());
  }
  Save(uid);
  return util::Error::Success();
}

util::Error UserProfileCache::SetRatio(UserID uid, const std::string& value)
{
  util::Error ok = Ensure(uid);
  if (!ok) return ok;
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  try
  {
    int ratio = boost::lexical_cast<int>(value);
    ok = instance.byUID[uid]->SetRatio(ratio);
  }
  catch (const boost::bad_lexical_cast& e)
  {
    return util::Error::Failure("Failed to parse as a number!");
  }
  Save(uid);
  return ok;
}

util::Error UserProfileCache::SetWeeklyAllotment(UserID uid, const std::string& value)
{
  util::Error ok = Ensure(uid);
  if (!ok) return ok;
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  try
  {
    int weeklyAllotment = boost::lexical_cast<int>(value);
    ok = instance.byUID[uid]->SetWeeklyAllotment(weeklyAllotment);
  }
  catch (const boost::bad_lexical_cast& e)
  {
    return util::Error::Failure("Failed to parse as a number!");
  }
  Save(uid);
  return ok;
}

util::Error UserProfileCache::SetHomeDir(UserID uid, const std::string& value)
{
  util::Error ok = Ensure(uid);
  if (!ok) return ok;
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ok = instance.byUID[uid]->SetHomeDir(value);
  Save(uid);
  return ok;
}

util::Error UserProfileCache::SetStartupDir(UserID uid, const std::string& value)
{
  util::Error ok = Ensure(uid);
  if (!ok) return ok;
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ok = instance.byUID[uid]->SetStartupDir(value);
  Save(uid);
  return ok;
}

util::Error UserProfileCache::SetIdleTime(UserID uid, const std::string& value)
{
  util::Error ok = Ensure(uid);
  if (!ok) return ok;
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  try
  {
    int idleTime = boost::lexical_cast<int>(value);
    ok = instance.byUID[uid]->SetIdleTime(idleTime);
  }
  catch (const boost::bad_lexical_cast& e)
  {
    return util::Error::Failure("Failed to parse as a number!");
  }
  Save(uid);
  return ok;
}

util::Error UserProfileCache::SetExpires(UserID uid, const std::string& value)
{
  util::Error ok = Ensure(uid);
  if (!ok) return ok;
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ok = instance.byUID[uid]->SetExpires(value);
  Save(uid);
  return ok;
}

util::Error UserProfileCache::SetNumLogins(UserID uid, const std::string& value)
{
  util::Error ok = Ensure(uid);
  if (!ok) return ok;
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  try
  {
    int numLogins = boost::lexical_cast<int>(value);
    ok = instance.byUID[uid]->SetNumLogins(numLogins);
  }
  catch (const boost::bad_lexical_cast& e)
  {
    return util::Error::Failure("Failed to parse as a number!");
  }
  Save(uid);
  return ok;
}

util::Error UserProfileCache::SetTagline(UserID uid, const std::string& value)
{
  util::Error ok = Ensure(uid);
  if (!ok) return ok;
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ok = instance.byUID[uid]->SetTagline(value);
  Save(uid);
  return ok;
}

util::Error UserProfileCache::SetComment(UserID uid, const std::string& value)
{
  util::Error ok = Ensure(uid);
  if (!ok) return ok;
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ok = instance.byUID[uid]->SetComment(value);
  Save(uid);
  return ok;
}

util::Error UserProfileCache::SetMaxDlSpeed(UserID uid, const std::string& value)
{
  util::Error ok = Ensure(uid);
  if (!ok) return ok;
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  try
  {
    int maxDlSpeed = boost::lexical_cast<int>(value);
    ok = instance.byUID[uid]->SetMaxDlSpeed(maxDlSpeed);
  }
  catch (const boost::bad_lexical_cast& e)
  {
    return util::Error::Failure("Failed to parse as a number!");
  }
  Save(uid);
  return ok;
}

util::Error UserProfileCache::SetMaxUlSpeed(UserID uid, const std::string& value)
{
  util::Error ok = Ensure(uid);
  if (!ok) return ok;
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  try
  {
    int maxUlSpeed = boost::lexical_cast<int>(value);
    ok = instance.byUID[uid]->SetMaxUlSpeed(maxUlSpeed);
  }
  catch (const boost::bad_lexical_cast& e)
  {
    return util::Error::Failure("Failed to parse as a number!");
  }
  Save(uid);
  return ok;
}

util::Error UserProfileCache::SetMaxSimDl(UserID uid, const std::string& value)
{
  util::Error ok = Ensure(uid);
  if (!ok) return ok;
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  try
  {
    int maxSimDl = boost::lexical_cast<int>(value);
    ok = instance.byUID[uid]->SetMaxSimDl(maxSimDl);
  }
  catch (const boost::bad_lexical_cast& e)
  {
    return util::Error::Failure("Failed to parse as a number!");
  }
  Save(uid);
  return ok;
}

util::Error UserProfileCache::SetMaxSimUl(UserID uid, const std::string& value)
{
  util::Error ok = Ensure(uid);
  if (!ok) return ok;
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  try
  {
    int maxSimUl = boost::lexical_cast<int>(value);
    ok = instance.byUID[uid]->SetMaxSimUl(maxSimUl);
  }
  catch (const boost::bad_lexical_cast& e)
  {
    return util::Error::Failure("Failed to parse as a number!");
  }
  Save(uid);
  return ok;
}


// end
}
