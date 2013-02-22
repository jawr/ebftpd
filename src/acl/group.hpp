#ifndef __ACL_GROUP_HPP
#define __ACL_GROUP_HPP

#include <string>
#include "acl/types.hpp"
#include "db/dbproxy.hpp"

namespace db
{
class Group;
}

namespace acl
{

struct GroupData
{
  acl::GroupID id;
  std::string name;

  std::string description;
  std::string comment;
  
  int slots;
  int leechSlots;
  int allotmentSlots;
  long long maxAllotmentSize;
  int maxLogins;

  GroupData();
};

typedef db::DBProxy<GroupData, acl::GroupID, db::Group> GroupProxy;

class Group
{
  GroupData data;
  GroupProxy db;
  
  Group();
  Group(GroupData&& data_);
  
public:
  Group& operator=(Group&& rhs);
  Group& operator=(const Group& rhs);
  
  Group(Group&& other);
  Group(const Group& other);
  
  ~Group();

  acl::GroupID ID() const { return data.id; }
  
  const std::string& Name() const { return data.name; }
  bool Rename(const std::string& name);

  const std::string& Description() const { return data.description; }
  void SetDescription(const std::string& description);
  
  const std::string& Comment() const { return data.comment; }
  void SetComment(const std::string& comment);

  int Slots() const { return data.slots; }
  void SetSlots(int slots);
  
  int LeechSlots() const { return data.leechSlots; }
  void SetLeechSlots(int leechSlots);
  
  int AllotmentSlots() const { return data.allotmentSlots; }
  void SetAllotmentSlots(int allotmentSlots);
  
  long long MaxAllotmentSize() const { return data.maxAllotmentSize; }
  void SetMaxAllotmentSize(long long maxAllotmentSize);
  
  int MaxLogins() const { return data.maxLogins; }
  void SetMaxLogins(int maxLogins);
  
  long long NumMembers() const;
  
  void Purge();
  
  static boost::optional<Group> Load(acl::GroupID gid);
  static boost::optional<Group> Load(const std::string& name);
  static boost::optional<Group> Create(const std::string& name);

  static std::vector<acl::GroupID> GetGIDs(const std::string& multiStr = "*");
  static std::vector<acl::Group> GetGroups(const std::string& multiStr = "*");
};

std::string GIDToName(acl::GroupID gid);
acl::GroupID NameToGID(const std::string& name);

inline bool GIDExists(acl::GroupID gid)
{ return GIDToName(gid) != "unknown"; }

inline bool GroupExists(const std::string& name)
{ return NameToGID(name) != -1; }


} /* acl namespace */

#endif
