#ifndef __ACL_GROUP_HPP
#define __ACL_GROUP_HPP

#include <string>
#include "acl/types.hpp"

namespace db { namespace bson
{
struct Group;
}
}

namespace acl
{

class Group
{
  std::string name;
  GroupID gid;
  
public:
  Group(const std::string& name, GroupID gid) : name(name), gid(gid) { }
  
  const std::string& Name() const { return name; }
  void SetName(const std::string& name) { this->name = name; }
  
  GroupID GID() const { return gid; }
  
  friend struct db::bson::Group;
};

} /* acl namespace */

#endif
