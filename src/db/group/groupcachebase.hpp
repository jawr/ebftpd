#ifndef __DB_GROUPCACHEBASE_HPP
#define __DB_GROUPCACHEBASE_HPP

#include <string>
#include "acl/types.hpp"

namespace db
{

struct GroupCacheBase
{
  virtual ~GroupCacheBase() { }
  virtual std::string GIDToName(acl::GroupID gid) = 0;
  virtual acl::GroupID NameToGID(const std::string& name) = 0;
};

} /* db namespace */

#endif
