#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include "acl.hpp"
#include "user.hpp"

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
  // unable to convert groupname to gid and vice versa yet!!
  return boost::indeterminate;
}

bool ACL::Check(const User& user)
{
  for (boost::ptr_vector<Permission>::const_iterator it =
       perms.begin(); it != perms.end(); ++it)
  {
    boost::tribool result = it->Evaluate(user);
    if (!boost::indeterminate(result)) return result;
  }
  return false;
}

ACL ACL::FromString(const std::string& str)
{
  ACL acl;
  
  std::vector<std::string> args;
  boost::split(args, str, boost::is_any_of(" "), boost::token_compress_on);
  for (std::vector<std::string>::const_iterator it =
       args.begin(); it != args.end(); ++it)
  {
    const std::string& arg = *it;
    bool negate = arg[0] == '!';
    if (arg[negate] == '-') // user permission
    {
      acl.perms.push_back(new UserPermission(arg.substr(1 + negate), negate));
    }
    else
    if (arg[negate] == '=') // group permission
    {
      acl.perms.push_back(new GroupPermission(arg.substr(1 + negate), negate));
    }
    else // flag permission
    {
      acl.perms.push_back(new FlagPermission(arg.substr(negate), negate));
    }
  }
  
  return acl;
}

} /* acl namespace */


#ifdef ACL_ACL_TEST

#include <iostream>

int main()
{
  acl::User u("bioboy", "password", "12");
  acl::ACL acl = acl::ACL::FromString("!61 =test -test");
  std::cout << acl.Check(u) << std::endl;
  acl = acl::ACL::FromString("!6 =test -someone");
  std::cout << acl.Check(u) << std::endl;
  acl = acl::ACL::FromString("!6 =test -bioboy");
  std::cout << acl.Check(u) << std::endl;
  
}

#endif
