#ifndef __DB_USER_UTIL_HPP
#define __DB_USER_UTIL_HPP

#include <string>
#include <memory>
#include "acl/types.hpp"

namespace db
{

struct UserCache
{
  virtual std::string UIDToName(acl::UserID uid) = 0;
  virtual acl::UserID NameToUID(const std::string& name) = 0;
};

void RegisterUserCache(const std::shared_ptr<UserCache>& cache);

std::string UIDToName(acl::UserID uid);
acl::UserID NameToUID(const std::string& name);

} /* db namespace */

#endif