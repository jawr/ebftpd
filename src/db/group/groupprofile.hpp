#ifndef __DB_GROUP_GROUPPROFILE_HPP
#define __DB_GROUP_GROUPPROFILE_HPP

#include <string>
#include <mongo/client/dbclient.h>
#include "acl/types.hpp"
#include "acl/groupprofile.hpp"

namespace db { namespace groupprofile
{

acl::GroupProfile Get(acl::GroupID gid);
void Set(acl::GroupID gid, mongo::BSONObj obj); 

void  SetSlots(acl::GroupID gid, int slots); 
void  SetLeechSlots(acl::GroupID gid, int slots); 
void  SetAllotSlots(acl::GroupID gid, int slots); 
void  SetMaxAllotSlots(acl::GroupID gid, int slots); 
void  SetMaxLogins(acl::GroupID gid, int maxLogins); 
void  SetDescription(acl::GroupID gid, const std::string& description);
void  SetComment(acl::GroupID gid, const std::string& comment);

}
}
#endif


