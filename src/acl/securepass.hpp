#ifndef __ACL_SECUREPASS_HPP
#define __ACL_SECUREPASS_HPP

#include <string>

namespace acl
{

class PasswdStrength;
class User;

bool SecurePass(const User& user, const std::string& password, PasswdStrength& strength);

} /* acl namespace */

#endif
