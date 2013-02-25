#ifndef __DB_UTIL_HPP
#define __DB_UTIL_HPP

#include <functional>
#include "acl/types.hpp"

namespace db
{

bool Initialise(const std::function<void(acl::UserID)>& userUpdatedCB);

} /* db namespace */

#endif
