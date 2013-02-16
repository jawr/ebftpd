#ifndef __DB_USERP_HPP
#define __DB_USERP_HPP

#include <string>
#include <memory>
#include <boost/optional.hpp>
#include "acl/types.hpp"

namespace mongo
{
class BSONObj;
}

namespace acl
{
class User;
} 

namespace db
{

class User
{
  acl::User& user;

  void SaveField(const std::string& field);
  
public:
  User(acl::User& user) :  user(user) { }
  
  acl::UserID Create();
  void SaveName();
  void SaveIPMasks();
  void SavePassword();
  void SaveFlags();
  void SavePrimaryGID();
  void SaveSecondaryGIDs();
  void SaveGadminGIDs();
  void SaveWeeklyAllotment();
  void SaveHomeDir();
  void SaveIdleTime();
  void SaveExpires();
  void SaveNumLogins();
  void SaveComment();
  void SaveTagline();
  void SaveMaxDownSpeed();
  void SaveMaxUpSpeed();
  void SaveMaxSimDown();
  void SaveMaxSimUp();
  void SaveLoggedIn();
  void SaveLastLogin();
  void SaveRatio();
  void IncrCredits(const std::string& section, long long kBytes, 
                   long long& newCredits);
  bool DecrCredits(const std::string& section, long long kBytes, 
                   bool force, long long& newCredits);
                   
  static boost::optional<acl::User> Load(acl::UserID uid);
};

mongo::BSONObj Serialize(const acl::User& user);
acl::User Unserialize(const mongo::BSONObj& obj);

struct UserCache
{
  virtual std::string UIDToName(acl::UserID uid) = 0;
  virtual acl::UserID NameToUID(const std::string& name) = 0;
};

void RegisterUserCache(const std::shared_ptr<UserCache>& cache);

std::string UIDToName(acl::UserID uid);
acl::UserID NameToUID(const std::string& name);

} /* db namespace */

#endif
