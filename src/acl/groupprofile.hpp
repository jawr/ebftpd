#ifndef __ACL_GROUPPROFILE_HPP
#define __ACL_GROUPPROFILE_HPP

#include "acl/types.hpp"
#include "util/error.hpp"

namespace db { namespace bson
{
  class GroupProfile;
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
  int allotSize;
  int maxLogins;
  
  std::string comment;
  
public:
  GroupProfile() : 
    gid(-1),
    description("ebftpd group profile")
  {}
  GroupProfile(acl::GroupID gid) : gid(gid),
    description("ebftpd group profile") {}
  

  util::Error SetDescription(const std::string& description);
  util::Error SetSlots(int slots);
  util::Error SetLeechSlots(int slots);
  util::Error SetAllotSlots(int slots);
  util::Error SetAllotSize(int size);
  util::Error SetMaxLogins(int maxLogins);
  util::Error SetComment(const std::string& comment);

  const std::string& Description() const { return description; }
  int Slots() const { return slots; }
  int LeechSlots() const { return leechSlots; }
  int AllotSlots() const { return allotSlots; }
  int AllotSize() const { return allotSize; }
  int MaxLogins() const { return maxLogins; }
  const std::string& Comment() const { return comment; }

  friend class db::bson::GroupProfile;
};

}
#endif
