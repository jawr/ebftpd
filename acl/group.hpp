#ifndef __ACL_GROUP_HPP
#define __ACL_GROUP_HPP

#include <string>
#include "acl/types.hpp"

namespace acl
{

class Group
{
  std::string name;
  gid_t gid;
  
public:
  Group(const std::string& name) : name(name), gid(-1) { }
  
  const std::string& Name() const { return name; }
  void SetName(const std::string& name) { this->name = name; }
  
  gid_t GID() const { return gid; }
};

} /* acl namespace */

#endif
