#ifndef __ACL_GROUP_HPP
#define __ACL_GROUP_HPP

#include <functional>
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "acl/types.hpp"
#include "util/keybase.hpp"
#include "acl/recode/dbproxy.hpp"
#include "acl/recode/groupdb.hpp"

namespace db { namespace recode
{
class GroupDB;
}
}

namespace acl { namespace recode
{

class GroupProfile;
class Group;

typedef DBProxy<Group, acl::GroupID, db::recode::GroupDB> GroupDBProxy;

class Group
{
  GroupDBProxy db;
  
  Group(const GroupProfile&) = delete;
  Group& operator=(const GroupProfile&) = delete;
  
protected:
  boost::posix_time::ptime modified;
  acl::GroupID id;
  std::string name;
  
public:
  class SetIDKey : util::KeyBase {  SetIDKey() { } };

  Group();
  Group(const std::string& name);
  
  virtual ~Group();
  
  acl::GroupID ID() const { return id; }
  void SetID(acl::GroupID id, const SetIDKey& key) { this->id = id; }
  
  const std::string& Name() const { return name; }
  void Rename(const std::string& name);

  friend class DBProxy<Group, acl::GroupID, db::recode::GroupDB>;
  friend class db::recode::GroupDB;
};

void GroupUpdatedSlot(const std::function<void(acl::GroupID)>& slot);

} /* recode namespace */
} /* acl namespace */

#endif
