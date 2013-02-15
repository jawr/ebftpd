#ifndef __DB_USERPROFILE_HPP
#define __DB_USERPROFILE_HPP

#include <string>
#include <boost/optional.hpp>
#include "acl/types.hpp"

namespace mongo
{
class BSONObj;
}

namespace acl
{
class UserProfile;
} 

namespace db
{

class UserProfile
{
  acl::UserProfile& profile;

  void SaveField(const std::string& field);
  
public:
  UserProfile(acl::UserProfile& profile) :  profile(profile) { }
  
  void Save();
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
                   
  static boost::optional<acl::UserProfile> Load(acl::UserID uid);
};

mongo::BSONObj Serialize(const acl::UserProfile& profile);
acl::UserProfile Unserialize(const mongo::BSONObj& obj);

} /* db namespace */

#endif
