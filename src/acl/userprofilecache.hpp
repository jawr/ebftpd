#ifndef __ACL_USERPROFILECACHE_HPP
#define __ACL_USERPROFILECACHE_HPP

#include <utility>
#include <unordered_map>
#include <boost/thread/mutex.hpp>
#include "acl/user.hpp"
#include "util/error.hpp"

namespace acl
{

class UserProfileCache
{
  typedef std::unordered_map<UserID, acl::UserProfile*> ByUIDMap;
  
  mutable boost::mutex mutex;
  ByUIDMap byUID;
  
  static UserProfileCache instance;
  static bool initalized;
  
  ~UserProfileCache();
  
  static void Save(const acl::UserProfile& user);

  static util::Error Ensure(UserID uid); 
  
public:
  static util::Error SetRatio(UserID uid, int8_t ratio);
  static util::Error SetWeeklyAllotment(UserID uid, signed int weeklyAllotment);
  static util::Error SetHomeDir(UserID uid, const std::string& homeDir);
  static util::Error SetStartupDir(UserID uid, const std::string& startupDir);
  static util::Error SetIdleTime(UserID uid, signed int idleTime);
  static util::Error SetExpires(UserID uid, const std::string& date);
  static util::Error SetNumLogins(UserID uid, signed int numLogins);
  static util::Error SetTagline(UserID uid, const std::string& tagline);
  static util::Error SetComment(UserID uid, const std::string& comment);
  static util::Error SetMaxDlSpeed(UserID uid, int maxDlSpeed);
  static util::Error SetMaxUlSpeed(UserID uid, int maxUlSpeed);
  static util::Error SetMaxSimDl(UserID uid, int maxSimDl);
  static util::Error SetMaxSimUl(UserID uid, int maxSimUl);

  static void Initalize();
  
  // these return const as the user objects should NEVER
  // be modified except via the above functions'
  static acl::UserProfile UserProfile(const std::string& name);
  static acl::UserProfile UserProfile(UserID uid);
};

} /* acl namespace */

#endif
