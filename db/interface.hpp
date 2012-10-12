#ifndef __DB_INTERFACE_HPP
#define __DB_INTERFACE_HPP

#include <mongo/client/dbclient.h>
#include <vector>
#include "acl/types.hpp"
#include "acl/user.hpp"
#include "acl/group.hpp"
#include "stats/stat.hpp"

namespace db 
{
  // initalize
  void Initalize(); // should only be called once by initial thread
  
  // user functions
  acl::UserID GetNewUserID();
  void SaveUser(const acl::User& user);    
  void GetUsers(std::vector<acl::User*>& users);
  void AddIpMask(const acl::User& user, const std::string& mask);
  void DelIpMask(const acl::User& user, const std::string& mask);
  void GetIpMasks(acl::UserMaskMap& userMaskMap);

  // group functions
  acl::GroupID GetNewGroupID();
  void SaveGroup(const acl::Group& group);
  void GetGroups(std::vector<acl::Group*>& groups);

  // stats functions
  void IncrementStats(const acl::User& user,
    long  long kbytes, double xfertime, stats::Direction direction);
  void DecrementStats(const acl::User& user,
    long long kbytes, double xfertime, stats::Direction direction);


// end
}
#endif
