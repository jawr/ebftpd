#define __ACL_FLAGS_CPP
#include "acl/flags.hpp"

namespace acl
{

const std::string validFlags = "12345678ABCDEFGHIJKLMN";

bool ValidFlags(const std::string& flags)
{
  for (char flag : flags)
    if (validFlags.find(flag) == std::string::npos) return false;
    
  return true;
}

} /* acl namespace */
