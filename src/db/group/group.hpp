#ifndef __DB_GROUP_GROUP_HPP
#define __DB_GROUP_GROUP_HPP

#include "acl/types.hpp"
#include "acl/group.hpp"

namespace db { namespace group
{
  acl::GroupID GetNewGroupID();
  void Save(const acl::Group& group);
  void GetAll(std::vector<acl::Group*>& groups);

// end
}
}

#endif
