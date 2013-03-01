#include "util/string.hpp"
#include "acl/acl.hpp"
#include "acl/user.hpp"

namespace acl
{

ACL::ACL(const std::string& s)
{
  FromString(s);
}

bool ACL::Evaluate(const ACLInfo& info) const
{
  if (finalResult) return *finalResult;
  for (const Permission& p : perms)
  {
    boost::tribool result = p.Evaluate(info);
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

void ACL::FromString(const std::string& str)
{
  std::vector<std::string> args;
  util::Split(args, str, " ", true);
  for (const auto& arg : args) FromStringArg(arg);
}

} /* acl namespace */
