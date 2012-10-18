#ifndef __DB_GROUP_GROUP_HPP
#define __DB_GROUP_GROUP_HPP

#include "acl/types.hpp"
#include "acl/group.hpp"

namespace db
{
  acl::GroupID GetNewGroupID();
  void SaveGroup(const acl::Group& group);
  void GetGroups(std::vector<acl::Group*>& groups);
}

#endif
