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
class UserData;
class User;
} 

namespace db
{

class User
{
  acl::UserData& user;

  void SaveField(const std::string& field);
  
public:
  User(acl::UserData& user) :  user(user) { }
  
  acl::UserID Create();
  bool SaveName();
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
  void IncrCredits(const std::string& section, long long kBytes);
  bool DecrCredits(const std::string& section, long long kBytes, bool force);
  
  void Purge() const;
  
  static boost::optional<acl::UserData> Load(acl::UserID uid);
};

struct UserCache
{
  virtual ~UserCache() { }
  virtual std::string UIDToName(acl::UserID uid) = 0;
  virtual acl::UserID NameToUID(const std::string& name) = 0;
  virtual acl::GroupID UIDToPrimaryGID(acl::UserID uid) = 0;
};

void RegisterUserCache(const std::shared_ptr<UserCache>& cache);

std::string UIDToName(acl::UserID uid);
acl::UserID NameToUID(const std::string& name);
acl::GroupID UIDToPrimaryGID(acl::UserID uid);

std::vector<acl::UserID> GetUIDs(const std::string& multiStr = "*");
std::vector<acl::UserData> GetUsers(const std::string& multiStr = "*");

} /* db namespace */

#endif
