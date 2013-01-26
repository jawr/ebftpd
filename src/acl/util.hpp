#ifndef __ACL_UTIL_HPP
#define __ACL_UTIL_HPP

#include <string>

namespace acl
{

void CreateDefaults();

enum class ValidationType
{
  Username,
  Groupname,
  Tagline
};

bool Validate(ValidationType type, const std::string& s);

} /* acl namespace */

#endif
