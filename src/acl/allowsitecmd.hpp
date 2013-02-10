#ifndef __ACL_ALLOWSITECMD_HPP
#define __ACL_ALLOWSITECMD_HPP

#include <string>

namespace acl
{

class User;

bool AllowSiteCmd(const User& user, const std::string& keyword);

} /* acl namespace */

#endif
