#ifndef __ACL_GROUPPROFILE_HPP
#define __ACL_GROUPPROFILE_HPP

#include "acl/types.hpp"
#include "util/error.hpp"

namespace db { namespace bson
{
struct GroupProfile;
}
}

namespace acl
{

class GroupProfile
{
  GroupID gid;
  std::string description;

  int slots;
  int leechSlots;
  int allotSlots;
  long long maxAllotSize;
  int maxLogins;
  
  std::string comment;
  
public:
  GroupProfile() : 
    gid(-1),
    slots(0),
    leechSlots(0),
    allotSlots(0),
    maxAllotSize(0),
    maxLogins(-1)
  { }
  
  GroupProfile(acl::GroupID gid) : 
    gid(gid),
    slots(0),
    leechSlots(0),
    allotSlots(0),
    maxAllotSize(0),
    maxLogins(-1)
  { }

  GroupID GID() const { return gid; }
  const std::string& Description() const { return description; }
  int Slots() const { return slots; }
  int LeechSlots() const { return leechSlots; }
  int AllotSlots() const { return allotSlots; }
  long long MaxAllotSize() const { return maxAllotSize; }
  int MaxLogins() const { return maxLogins; }
  const std::string& Comment() const { return comment; }

  friend struct db::bson::GroupProfile;
};

}
#endif
