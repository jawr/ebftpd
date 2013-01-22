#ifndef __ACL_USERCACHE_HPP
#define __ACL_USERCACHE_HPP

#include <utility>
#include <unordered_map>
#include <boost/thread/mutex.hpp>
#include "acl/user.hpp"
#include "util/error.hpp"

namespace acl
{

class UserCache
{
  typedef std::unordered_map<std::string, acl::User*> ByNameMap;
  typedef std::unordered_map<UserID, acl::User*> ByUIDMap;
  
  mutable boost::mutex mutex;
  ByNameMap byName;
  ByUIDMap byUID;
  
  static UserCache instance;
  static bool initialized;
  
  ~UserCache();
  
  static void Save(const acl::User& user);
  static void Save(const acl::User& user, const std::string& field);
  
public:
  static bool Exists(const std::string& name);
  static bool Exists(UserID uid);
  static util::Error Create(const std::string& name, const std::string& password,
                     const std::string& flags, const UserID creator);
  static util::Error Delete(const std::string& name);
  static util::Error Purge(const std::string& name);
  static util::Error Readd(const std::string& name);
  static util::Error Rename(const std::string& oldName, const std::string& newName);
  static util::Error SetPassword(const std::string& name, const std::string& password);
  static util::Error SetFlags(const std::string& name, const std::string& flags);
  static util::Error AddFlags(const std::string& name, const std::string& flags);
  static util::Error DelFlags(const std::string& name, const std::string& flags);
  static util::Error SetPrimaryGID(const std::string& name, GroupID primaryGID, GroupID oldPprimaryGID);
  static util::Error SetPrimaryGID(const std::string& name, GroupID primaryGID)
  {
    GroupID oldGID = -1;
    return SetPrimaryGID(name, primaryGID, oldGID);
  }
  static util::Error AddGID(const std::string& name, GroupID secondaryGID);
  static util::Error DelGID(const std::string& name, GroupID secondaryGID);
  static util::Error ResetGIDs(const std::string& name);
  static GroupID PrimaryGID(UserID uid);
  
  static util::Error IncrCredits(const std::string& name, long long kbytes);
  static util::Error DecrCredits(const std::string& name, long long kbytes);

  static util::Error SetTagline(const std::string& name, const std::string& tagline);
  
  static unsigned Count(bool includeDeleted = false);

  static void Initialize();
  
  // these return const as the user objects should NEVER
  // be modified except via the above functions'
  static acl::User User(const std::string& name);
  static acl::User User(UserID uid);
  
  static UserID NameToUID(const std::string& name);
  static std::string UIDToName(UserID uid);
};

} /* acl namespace */

#endif
