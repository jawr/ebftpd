#ifndef __DB_USERUTIL_HPP
#define __DB_USERUTIL_HPP

#include <memory>
#include <string>
#include <vector>
#include "acl/types.hpp"

namespace acl
{
struct UserData;
}

namespace db
{

class Connection;
struct UserCacheBase;

void SetUserCache(const std::shared_ptr<UserCacheBase>& cache);

std::string UIDToName(acl::UserID uid);
acl::UserID NameToUID(const std::string& name);
acl::GroupID UIDToPrimaryGID(acl::UserID uid);

bool IdentIPAllowed(const std::string& identAddress);
bool IdentIPAllowed(const std::string& identAddress, acl::UserID uid);

std::vector<std::string> LookupIPMasks(Connection& conn, acl::UserID uid = -1);

} /* db namespace */

#endif
