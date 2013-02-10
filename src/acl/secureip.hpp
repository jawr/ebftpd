#ifndef __ACL_SECUREIP_HPP
#define __ACL_SECUREIP_HPP

#include <string>

namespace acl
{

class IPStrength;
class User;

bool SecureIP(const User& user, const std::string& ip, IPStrength& minimum);

} /* acl namespace */

#endif
