#ifndef __ACL_GROUP_HPP
#define __ACL_GROUP_HPP

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "acl/types.hpp"
#include "db/dbproxy.hpp"
#include "util/keybase.hpp"

namespace mongo
{
class BSONObj;
}

namespace acl
{
class Group;
}

namespace db
{
class Group;
mongo::BSONObj Serialize(const acl::Group& group);
acl::Group Unserialize(const mongo::BSONObj& obj);
}

namespace acl
{

typedef DBProxy<Group, acl::GroupID, db::Group> GroupProxy;

class Group
{
  GroupProxy db;
  
  boost::posix_time::ptime modified;
  acl::GroupID id;
  std::string name;

  std::string description;
  std::string comment;
  
  int slots;
  int leechSlots;
  int allotmentSlots;
  long long maxAllotmentSize;
  int maxLogins;

  Group();
  
public:
  class SetIDKey : util::KeyBase {  SetIDKey() { } };
  
  acl::GroupID ID() const { return id; }
  void SetID(acl::GroupID id, const SetIDKey& key) { this->id = id; }
  
  const std::string& Name() const { return name; }
  void Rename(const std::string& name);

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
  
  static boost::optional<Group> Load(acl::GroupID gid);
  static Group Skeleton(acl::GroupID gid);
  static Group Create(const std::string& name);
  
  friend class DBProxy<Group, acl::GroupID, db::Group>;
  friend class db::Group;
  friend mongo::BSONObj db::Serialize(const Group& group);
  friend Group db::Unserialize(const mongo::BSONObj& obj);
};

std::string GIDToName(acl::GroupID gid);
acl::GroupID NameToGID(const std::string& name);

inline bool GIDExists(acl::GroupID gid)
{ return GIDToName(gid) == "unknown"; }

inline bool GroupExists(const std::string& name)
{ return NameToGID(name) != -1; }


} /* acl namespace */

#endif
