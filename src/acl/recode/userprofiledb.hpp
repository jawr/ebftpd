#ifndef __DB_USERPROFILEDB_HPP
#define __DB_USERPROFILEDB_HPP

#include <iostream>
namespace acl { namespace recode
{
class User;
} 
} 

namespace db { namespace recode
{

class UserProfileDB
{
  acl::recode::UserProfile& profile;
  
public:
  UserProfileDB(acl::recode::UserProfile& profile) :  profile(profile) { }
  
  void Save() { std::cout << "Save" << std::endl; }
  void SavePassword() { std::cout << "SavePassword" << std::endl; }
  void SaveFlags() { std::cout << "SaveFlags" << std::endl; }
  void SavePrimaryGID() { std::cout << "SavePrimaryGID" << std::endl; }
  void SaveSecondaryGIDs() { std::cout << "SaveSecondaryGIDs" << std::endl; }
  void SaveGadminGIDs() { std::cout << "SaveGadminGIDs" << std::endl; }
  void SaveWeeklyAllotment() { std::cout << "SaveWeeklyAllotment" << std::endl; }
  void SaveHomeDir() { std::cout << "SaveHomeDir" << std::endl; }
  void SaveIdleTime() { std::cout << "SaveIdleTime" << std::endl; }
  void SaveExpires() { std::cout << "SaveExpires" << std::endl; }
  void SaveNumLogins() { std::cout << "SaveNumLogins" << std::endl; }
  void SaveComment() { std::cout << "SaveComment" << std::endl; }
  void SaveTagline() { std::cout << "SaveTagline" << std::endl; }
  void SaveMaxDownSpeed() { std::cout << "SaveMaxDownSpeed" << std::endl; }
  void SaveMaxUpSpeed() { std::cout << "SaveMaxUpSpeed" << std::endl; }
  void SaveMaxSimDown() { std::cout << "SaveMaxSimDown" << std::endl; }
  void SaveMaxSimUp() { std::cout << "SaveMaxSimUp" << std::endl; }
  void SaveLoggedIn() { std::cout << "SaveLoggedIn" << std::endl; }
  void SaveRatio() { std::cout << "SaveRatio" << std::endl; }
  void IncrCredits(const std::string& section, long long kBytes, 
                   long long& newCredits)
  { std::cout << "IncrCredits" << std::endl; }
  virtual bool DecrCredits(const std::string& section, long long kBytes, 
                           bool force, long long& newCredits)
  { std::cout << "DecrCredits" << std::endl; }
};

} /* recode namespace */
} /* db namespace */

#endif
