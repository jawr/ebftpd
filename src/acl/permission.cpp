#include "acl/permission.hpp"
#include "acl/user.hpp"
#include "acl/acl.hpp"

namespace acl
{

boost::tribool FlagPermission::Evaluate(const ACLInfo& info) const
{
  if (flags.find('*') != std::string::npos) return !negate;
  for (char ch : info.flags)
  {
    if (flags.find(ch) != std::string::npos) return !negate;
  }
  return boost::indeterminate;
}

boost::tribool UserPermission::Evaluate(const ACLInfo& info) const
{
  if (info.username == username) return !negate;
  return boost::indeterminate;
}

boost::tribool GroupPermission::Evaluate(const ACLInfo& info) const
{
  if (info.groupname == groupname) return !negate;
  return boost::indeterminate;
}

} /* acl namespace */
