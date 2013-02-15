#ifndef __DB_GROUP_UTIL_HPP
#define __DB_GROUP_UTIL_HPP

#include <string>
#include <memory>
#include "acl/types.hpp"

namespace db
{

struct GroupCache
{
  virtual std::string GIDToName(acl::GroupID gid) = 0;
  virtual acl::GroupID NameToGID(const std::string& name) = 0;
};

void RegisterGroupCache(const std::shared_ptr<GroupCache>& cache);

std::string GIDToName(acl::GroupID gid);
acl::GroupID NameToGID(const std::string& name);

} /* db namespace */

#endif