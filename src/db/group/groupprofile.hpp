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

void SetSlots(acl::GroupID gid, int slots); 
void SetLeechSlots(acl::GroupID gid, int slots); 
void SetAllotSlots(acl::GroupID gid, int slots); 
void SetMaxAllotSize(acl::GroupID gid, long long allotment); 
void SetMaxLogins(acl::GroupID gid, int logins); 
void SetDescription(acl::GroupID gid, const std::string& description);
void SetComment(acl::GroupID gid, const std::string& comment);

}
}
#endif


