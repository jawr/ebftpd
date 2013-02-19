#ifndef __DB_IPMASKS_HPP
#define __DB_IPMASKS_HPP

#include <string>
#include <memory>
#include "acl/types.hpp"

namespace db
{

struct IPMaskCacheBase;

void RegisterIPMaskCache(const std::shared_ptr<IPMaskCacheBase>& cache);
bool IdentIPAllowed(const std::string& identAddress);
bool IdentIPAllowed(const std::string& identAddress, acl::UserID uid);

} /* db namespace */

#endif
