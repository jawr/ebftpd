#include "util/string.hpp"
#include "acl/acl.hpp"
#include "acl/user.hpp"

namespace acl
{

ACL& ACL::operator=(const ACL& rhs)
{
  perms.clear();
  for (const Permission* p : rhs.perms)
  {
    perms.emplace_back(p->Clone());
  }
  finalResult = rhs.finalResult;
  return *this;
}

ACL& ACL::operator=(ACL&& rhs)
{
  perms = std::move(rhs.perms);
  finalResult = std::move(rhs.finalResult);
  rhs.perms.clear();
  return *this;
}

ACL::ACL(const ACL& other) :
  finalResult(other.finalResult)
{
  for (const Permission* p : other.perms)
  {
    perms.emplace_back(p->Clone());
  }
}

ACL::ACL(ACL&& other) :
  perms(std::move(other.perms)),
  finalResult(std::move(other.finalResult))
{
  other.perms.clear();
}

ACL::~ACL()
{
  for (const Permission* p : perms)
    delete p;
}

bool ACL::Evaluate(const User& user) const
{
  if (finalResult) return *finalResult;
  for (const Permission* p : perms)
  {
    boost::tribool result = p->Evaluate(user);
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
  util::Split(args, str, " ", true);
  for (const auto& arg : args) acl.FromStringArg(arg);
  return acl;
}

} /* acl namespace */
