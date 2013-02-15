#ifndef __ACL_GROUPPROFILE_HPP
#define __ACL_GROUPPROFILE_HPP

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
class GroupProfile;
}

namespace db
{
class GroupProfile;
mongo::BSONObj Serialize(const acl::GroupProfile& profile);
acl::GroupProfile Unserialize(const mongo::BSONObj& obj);
}

namespace acl
{

typedef DBProxy<GroupProfile, acl::GroupID, db::GroupProfile> GroupProfileProxy;

class GroupProfile
{
  GroupProfileProxy db;
  
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

  GroupProfile();
  
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
  
  static boost::optional<GroupProfile> Load(acl::GroupID gid);
  static GroupProfile Skeleton(acl::GroupID gid);
  
  friend class DBProxy<GroupProfile, acl::GroupID, db::GroupProfile>;
  friend class db::GroupProfile;
  friend mongo::BSONObj db::Serialize(const GroupProfile& profile);
  friend GroupProfile db::Unserialize(const mongo::BSONObj& obj);
};

} /* acl namespace */

#endif
