#ifndef __DB_GROUPUTIL_HPP
#define __DB_GROUPUTIL_HPP

#include <string>
#include <memory>
#include "acl/types.hpp"

namespace db
{

struct GroupCacheBase;

void SetGroupCache(const std::shared_ptr<GroupCacheBase>& cache);

std::string GIDToName(acl::GroupID gid);
acl::GroupID NameToGID(const std::string& name);

} /* db namespace */

#endif
