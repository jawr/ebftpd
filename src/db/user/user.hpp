#ifndef __DB_USER_USER_HPP
#define __DB_USER_USER_HPP

#include "acl/types.hpp"
#include "acl/user.hpp"
#include "acl/userprofile.hpp"
#include "acl/ipmaskcache.hpp"


namespace db
{
  acl::UserID GetNewUserID();
  void SaveUser(const acl::User& user);    
  void DeleteUser(const acl::UserID& uid);
  void GetUsers(std::vector<acl::User*>& users);
  void AddIpMask(const acl::User& user, const std::string& mask);
  void DelIpMask(const acl::User& user, const std::string& mask);
  void GetIpMasks(acl::UserIPMaskMap& userIPMaskMap);

  void SaveUserProfile(const acl::UserProfile& profile);
  void GetUserProfiles(std::vector<acl::UserProfile*>& profiles);
}
#endif
