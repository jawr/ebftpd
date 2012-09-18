#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/bind.hpp>
#include "acl.hpp"
#include "user.hpp"

namespace acl
{

bool ACL::Check(const User& user)
{
  if (finalResult) return *finalResult;
  for (boost::ptr_vector<Permission>::const_iterator it =
       perms.begin(); it != perms.end(); ++it)
  {
    boost::tribool result = it->Evaluate(user);
    if (!boost::indeterminate(result))
    {
      finalResult.reset(result);
      return *finalResult;
    }
  }
  finalResult.reset(false);
  return *finalResult;
}

void ACL::FromStringArg(const std::string& arg)
{
  bool negate = arg[0] == '!';
  if (arg[negate] == '-') // user permission
  {
    perms.push_back(new UserPermission(arg.substr(1 + negate), negate));
  }
  else
  if (arg[negate] == '=') // group permission
  {
    perms.push_back(new GroupPermission(arg.substr(1 + negate), negate));
  }
  else // flag permission
  {
    perms.push_back(new FlagPermission(arg.substr(negate), negate));
  }
}

ACL ACL::FromString(const std::string& str)
{
  ACL acl;
  std::vector<std::string> args;
  boost::split(args, str, boost::is_any_of(" "), boost::token_compress_on);
  std::for_each(args.begin(), args.end(), boost::bind(&ACL::FromStringArg, &acl, _1));  
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
