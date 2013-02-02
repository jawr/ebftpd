#ifndef __DB_GROUP_GROUP_HPP
#define __DB_GROUP_GROUP_HPP

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/optional.hpp>
#include "acl/types.hpp"

namespace boost { namespace posix_time
{
class ptime;
}
}

namespace acl
{
class Group;
}

namespace db { namespace group
{

bool Create(acl::Group& group);
void Save(const acl::Group& group);
std::vector<acl::Group> GetAll();
boost::ptr_vector<acl::Group> 
GetAllPtr(const boost::optional<boost::posix_time::
          ptime>& modified = boost::none);
void Delete(const acl::GroupID gid);

// end
}
}

#endif
