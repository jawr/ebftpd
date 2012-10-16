#include "acl/userprofilecache.hpp"
#include "acl/userprofile.hpp"
#include "util/error.hpp"

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
  return util::Error::Success();
}

util::Error UserProfileCache::SetRatio(UserID uid, int8_t ratio)
{
  util::Error ok = Ensure(uid);
  if (!ok) return ok;
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  return instance.byUID[uid]->SetRatio(ratio);
}

util::Error UserProfileCache::SetWeeklyAllotment(UserID uid, signed int weeklyAllotment)
{
  util::Error ok = Ensure(uid);
  if (!ok) return ok;
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  return instance.byUID[uid]->SetWeeklyAllotment(weeklyAllotment);
}

util::Error UserProfileCache::SetHomeDir(UserID uid, const std::string& homeDir)
{
  util::Error ok = Ensure(uid);
  if (!ok) return ok;
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  return instance.byUID[uid]->SetHomeDir(homeDir);
}

util::Error UserProfileCache::SetStartupDir(UserID uid, const std::string& startupDir)
{
  util::Error ok = Ensure(uid);
  if (!ok) return ok;
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  return instance.byUID[uid]->SetStartupDir(startupDir);
}

util::Error UserProfileCache::SetIdleTime(UserID uid, signed int idleTime)
{
  util::Error ok = Ensure(uid);
  if (!ok) return ok;
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  return instance.byUID[uid]->SetIdleTime(idleTime);
}

util::Error UserProfileCache::SetExpires(UserID uid, const std::string& date)
{
  util::Error ok = Ensure(uid);
  if (!ok) return ok;
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  return instance.byUID[uid]->SetExpires(date);
}

util::Error UserProfileCache::SetNumLogins(UserID uid, signed int numLogins)
{
  util::Error ok = Ensure(uid);
  if (!ok) return ok;
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  return instance.byUID[uid]->SetNumLogins(numLogins);
}

util::Error UserProfileCache::SetTagline(UserID uid, const std::string& tagline)
{
  util::Error ok = Ensure(uid);
  if (!ok) return ok;
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  return instance.byUID[uid]->SetTagline(tagline);
}

util::Error UserProfileCache::SetComment(UserID uid, const std::string& comment)
{
  util::Error ok = Ensure(uid);
  if (!ok) return ok;
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  return instance.byUID[uid]->SetComment(comment);
}

util::Error UserProfileCache::SetMaxDlSpeed(UserID uid, int maxDlSpeed)
{
  util::Error ok = Ensure(uid);
  if (!ok) return ok;
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  return instance.byUID[uid]->SetMaxDlSpeed(maxDlSpeed);
}

util::Error UserProfileCache::SetMaxUlSpeed(UserID uid, int maxUlSpeed)
{
  util::Error ok = Ensure(uid);
  if (!ok) return ok;
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  return instance.byUID[uid]->SetMaxUlSpeed(maxUlSpeed);
}

util::Error UserProfileCache::SetMaxSimDl(UserID uid, int maxSimDl)
{
  util::Error ok = Ensure(uid);
  if (!ok) return ok;
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  return instance.byUID[uid]->SetMaxSimDl(maxSimDl);
}

util::Error UserProfileCache::SetMaxSimUl(UserID uid, int maxSimUl)
{
  util::Error ok = Ensure(uid);
  if (!ok) return ok;
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  return instance.byUID[uid]->SetMaxSimUl(maxSimUl);
}


// end
}
