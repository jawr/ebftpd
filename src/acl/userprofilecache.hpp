#ifndef __ACL_USERPROFILECACHE_HPP
#define __ACL_USERPROFILECACHE_HPP

#include <utility>
#include <unordered_map>
#include <boost/thread/mutex.hpp>
#include "acl/user.hpp"
#include "acl/userprofile.hpp"
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
  
  static void Save(const acl::UserID& uid);

  static util::Error Ensure(UserID uid); 
  
public:
  static util::Error Create(UserID uid, UserID creator);
  static util::Error SetRatio(UserID uid, const std::string& value);
  static util::Error SetWeeklyAllotment(UserID uid, const std::string& value);
  static util::Error SetHomeDir(UserID uid, const std::string& value);
  static util::Error SetStartupDir(UserID uid, const std::string& value);
  static util::Error SetIdleTime(UserID uid, const std::string& value);
  static util::Error SetExpires(UserID uid, const std::string& value);
  static util::Error SetNumLogins(UserID uid, const std::string& value);
  static util::Error SetTagline(UserID uid, const std::string& value);
  static util::Error SetComment(UserID uid, const std::string& value);
  static util::Error SetMaxDlSpeed(UserID uid, const std::string& value);
  static util::Error SetMaxUlSpeed(UserID uid, const std::string& value);
  static util::Error SetMaxSimDl(UserID uid, const std::string& value);
  static util::Error SetMaxSimUl(UserID uid, const std::string& value);

  static void Initalize();

  // these return const as the user objects should NEVER
  // be modified except via the above functions'
  static acl::UserProfile UserProfile(acl::UserID uid);
};

} /* acl namespace */

#endif
