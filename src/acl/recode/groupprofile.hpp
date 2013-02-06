#ifndef __ACL_GROUPPROFILE_HPP
#define __ACL_GROUPPROFILE_HPP

#include <string>
#include "acl/recode/group.hpp"

namespace db { namespace recode
{
class GroupProfileDB;
}
}

namespace acl { namespace recode
{

class GroupProfile;

typedef DBProxy<GroupProfile, acl::GroupID, db::recode::GroupProfileDB> GroupProfileDBProxy;

class GroupProfile : public Group
{
  GroupProfileDBProxy db;
  
  std::string description;
  std::string comment;
  
  int slots;
  int leechSlots;
  int allotmentSlots;
  long long maxAllotmentSize;
  int maxLogins;
  
public:
  GroupProfile();
  GroupProfile(const Group& group);
  
  ~GroupProfile();
  
  const std::string& Description() const { return description; }
  void SetDescription(const std::string& description);
  
  const std::string& Comment() const { return comment; }
  void SetComment(const std::string& comment);

  int Slots() const { return slots; }
  void SetSlots(int slots);
  
  int LeechSlots() const { return leechSlots; }
  void SetLeechslots(int leechSlots);
  
  int AllotmentSlots() const { return allotmentSlots; }
  void SetAllotmentSlots(int allotmentSlots);
  
  long long MaxAllotmentSize() const { return maxAllotmentSize; }
  void SetMaxAllotmentSize(long long maxAllotmentSize);
  
  int MaxLogins() const { return maxLogins; }
  void SetMaxLogins(int maxLogins);
  
  friend class DBProxy<GroupProfile, acl::GroupID, db::recode::GroupProfileDB>;
  friend class db::recode::GroupProfileDB;
};

} /* recode namespace */
} /* acl namespace */

#endif
