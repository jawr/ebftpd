#ifndef __DB_GROUP_GROUP_HPP
#define __DB_GROUP_GROUP_HPP

#include <boost/ptr_container/ptr_vector.hpp>
#include "acl/types.hpp"
#include "acl/group.hpp"

namespace db { namespace group
{

  acl::GroupID GetNewGroupID();
  void Save(const acl::Group& group);
  std::vector<acl::Group> GetAll();
  boost::ptr_vector<acl::Group> GetAllPtr();
  void Delete(const acl::GroupID gid);

// end
}
}

#endif
