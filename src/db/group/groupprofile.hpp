#ifndef __DB_GROUP_GROUPPROFILE_HPP
#define __DB_GROUP_GROUPPROFILE_HPP

#include <string>
#include <mongo/client/dbclient.h>
#include "acl/types.hpp"
#include "acl/groupprofile.hpp"

namespace db { namespace group
{
  acl::GroupProfile Get(const acl::GroupID& gid);
  
  void Set(const acl::GroupID& gid, mongo::BSONObj obj); 

  util::Error SetSlots(const acl::GroupID& gid, int slots); 
  util::Error SetLeechSlots(const acl::GroupID& gid, int slots); 
  util::Error SetAllotSlots(const acl::GroupID& gid, int slots); 
  util::Error SetMaxAllotSlots(const acl::GroupID& gid, int slots); 
  util::Error SetMaxLogins(const acl::GroupID& gid, int maxLogins); 
  util::Error SetDescription(const acl::GroupID& gid, 
    const std::string& description);
  util::Error SetComment(const acl::GroupID& gid, 
    const std::string& comment);

}
}
#endif


