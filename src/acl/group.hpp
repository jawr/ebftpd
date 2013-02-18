#ifndef __ACL_GROUP_HPP
#define __ACL_GROUP_HPP

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "acl/types.hpp"
#include "db/dbproxy.hpp"

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

template <typename T> mongo::BSONObj Serialize(const T& group);
template <> mongo::BSONObj Serialize<acl::Group>(const acl::Group& group);

template <typename T> T Unserialize(const mongo::BSONObj& obj);
template <> acl::Group Unserialize<acl::Group>(const mongo::BSONObj& obj);
}

namespace acl
{

typedef db::DBProxy<Group, acl::GroupID, db::Group> GroupProxy;

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
  ~Group();

  acl::GroupID ID() const { return id; }
  
  const std::string& Name() const { return name; }
  bool Rename(const std::string& name);

  const std::string& Description() const { return description; }
  void SetDescription(const std::string& description);
  
  const std::string& Comment() const { return comment; }
  void SetComment(const std::string& comment);

  int Slots() const { return slots; }
  void SetSlots(int slots);
  
  int LeechSlots() const { return leechSlots; }
  void SetLeechSlots(int leechSlots);
  
  int AllotmentSlots() const { return allotmentSlots; }
  void SetAllotmentSlots(int allotmentSlots);
  
  long long MaxAllotmentSize() const { return maxAllotmentSize; }
  void SetMaxAllotmentSize(long long maxAllotmentSize);
  
  int MaxLogins() const { return maxLogins; }
  void SetMaxLogins(int maxLogins);
  
  long long NumMembers() const;
  
  void Purge();
  
  static boost::optional<Group> Load(acl::GroupID gid);
  static boost::optional<Group> Load(const std::string& name);
  static boost::optional<Group> Create(const std::string& name);
  
  friend class db::DBProxy<Group, acl::GroupID, db::Group>;
  friend class db::Group;
  friend mongo::BSONObj db::Serialize<acl::Group>(const Group& group);
  friend Group db::Unserialize<acl::Group>(const mongo::BSONObj& obj);
};

std::string GIDToName(acl::GroupID gid);
acl::GroupID NameToGID(const std::string& name);

inline bool GIDExists(acl::GroupID gid)
{ return GIDToName(gid) == "unknown"; }

inline bool GroupExists(const std::string& name)
{ return NameToGID(name) != -1; }


} /* acl namespace */

#endif
