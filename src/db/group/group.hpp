#ifndef __DB_GROUP_GROUP_HPP
#define __DB_GROUP_GROUP_HPP

#include <boost/ptr_container/ptr_vector.hpp>
#include "acl/types.hpp"
#include "acl/group.hpp"

namespace db { namespace group
{

acl::GroupID GetNewGroupID();
void Save(const acl::Group& group);
void GetAll(boost::ptr_vector<acl::Group>& groups);
void Delete(const acl::GroupID gid);
// end
}
}

#endif
