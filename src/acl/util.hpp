#ifndef __ACL_UTIL_HPP
#define __ACL_UTIL_HPP

#include <string>

namespace acl
{

bool CreateDefaults();

enum class ValidationType
{
  Username,
  Groupname,
  Tagline
};

bool Validate(ValidationType type, const std::string& s);

class User;

std::string RatioString(const User& user);
std::string CreditString(const User& user);
std::string GroupString(const User& user);

} /* acl namespace */

#endif
