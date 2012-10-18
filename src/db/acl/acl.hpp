#ifndef __DB_ACL_ACL_HPP
#define __DB_ACL_ACL_HPP

#include <boost/ptr_container/ptr_vector.hpp>
#include "acl/types.hpp"
#include "acl/user.hpp"
#include "util/error.hpp"

namespace db
{
  util::Error GetUsersByACL(boost::ptr_vector<acl::User>& users, 
    std::string& acl);
}

#endif
