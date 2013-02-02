#ifndef __DB_GROUP_GROUPPROFILE_HPP
#define __DB_GROUP_GROUPPROFILE_HPP

#include <string>
#include "acl/types.hpp"

namespace mongo
{
class BSONObj;
}

namespace acl
{
class GroupProfile;
}

namespace util
{
class Error;
}

namespace db { namespace groupprofile
{

void Delete(acl::GroupID gid);
void Save(const acl::GroupProfile& profile);
std::vector<acl::GroupProfile> GetAll();
acl::GroupProfile Get(acl::GroupID gid);
void Set(acl::GroupID gid, mongo::BSONObj obj); 

util::Error SetSlots(acl::GroupID gid, const std::string& slots); 
util::Error SetLeechSlots(acl::GroupID gid, const std::string& slots); 
util::Error SetAllotSlots(acl::GroupID gid, const std::string& slots); 
util::Error SetMaxAllotSlots(acl::GroupID gid, const std::string& slots); 
util::Error SetMaxLogins(acl::GroupID gid, const std::string& maxLogins); 
util::Error SetDescription(acl::GroupID gid, 
  const std::string& description);
util::Error SetComment(acl::GroupID gid, 
  const std::string& comment);

}
}
#endif


