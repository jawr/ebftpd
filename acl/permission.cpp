#include "acl/permission.hpp"
#include "acl/user.hpp"

namespace acl
{

Permission* new_clone(Permission const& other)
{
  return other.Clone();
}

boost::tribool FlagPermission::Evaluate(const User& user) const
{
  if (flags.find('*') != std::string::npos) return !negate;
  if (user.CheckFlags(flags)) return !negate;
  return boost::indeterminate;
}

boost::tribool UserPermission::Evaluate(const User& user) const
{
  if (user.Name() == username) return !negate;
  return boost::indeterminate;
}

boost::tribool GroupPermission::Evaluate(const User& user) const
{
  (void) user;
  // unable to convert groupname to gid and vice versa yet!!
  return boost::indeterminate;
}

} /* acl namespace */
