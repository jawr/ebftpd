#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include "acl/acl.hpp"
#include "acl/user.hpp"

namespace acl
{

bool ACL::Evaluate(const User& user) const
{
  if (finalResult) return *finalResult;
  for (const Permission& p : perms)
  {
    boost::tribool result = p.Evaluate(user);
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
  for (const auto& arg : args) acl.FromStringArg(arg);
  return acl;
}

} /* acl namespace */
