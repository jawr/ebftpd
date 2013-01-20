#ifndef __DB_USER_IPMASK_HPP
#define __DB_USER_IPMASK_HPP

#include <vector>
#include "acl/types.hpp"
#include "acl/ipmaskcache.hpp"

namespace db { namespace ipmask
{

  void Add(acl::UserID uid, const std::string& mask);
  void Delete(acl::UserID uid, const std::string& mask);
  void GetAll(acl::UserIPMaskMap& userIPMaskMap);

// end
}
}
#endif
