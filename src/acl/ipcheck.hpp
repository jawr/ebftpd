#ifndef __ACL_IPCHECK_HPP
#define __ACL_IPCHECK_HPP

#include <string>
#include "acl/types.hpp"

namespace acl
{

inline bool IPAllowed(const std::string& address) { return true; }
inline bool IdentIPAllowed(acl::UserID uid, const std::string& identAddress) { return true; }

} /* acl namespace */

#endif
