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
  
  ~UserCache();
  
  static void Save(const acl::User& user);
  
public:
  static bool Exists(const std::string& name);
  static bool Exists(UserID uid);
  static util::Error Create(const std::string& name, const std::string& password,
                     const std::string& flags);
  static util::Error Delete(const std::string& name);
  static util::Error Rename(const std::string& oldName, const std::string& newName);
  static util::Error SetPassword(const std::string& name, const std::string& password);
  static util::Error SetFlags(const std::string& name, const std::string& flags);
  static util::Error AddFlags(const std::string& name, const std::string& flags);
  static util::Error DelFlags(const std::string& name, const std::string& flags);
  static util::Error SetPrimaryGID(const std::string& name, GroupID primaryGID);
  static util::Error AddSecondaryGID(const std::string& name, GroupID secondaryGID);
  static util::Error DelSecondaryGID(const std::string& name, GroupID secondaryGID);

  // might want to wrap this in an initalize, although it might be called seperately
  static void Sync();
  
  // these return const as the user objects should NEVER
  // be modified except via the above functions'
  static const acl::User User(const std::string& name);
  static const acl::User User(UserID uid);
};

} /* acl namespace */

#endif
