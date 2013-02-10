#include "acl/credits.hpp"
#include "fs/path.hpp"
#include "acl/user.hpp"
#include "cfg/get.hpp"
#include "util/string.hpp"

namespace acl
{

boost::optional<const cfg::setting::Creditcheck&> 
CreditCheck(const User& user, const fs::VirtualPath& path)
{
  for (const auto& cc : cfg::Get().Creditcheck())
  {
    if (util::string::WildcardMatch(cc.Path(), path.ToString()) &&
        cc.ACL().Evaluate(user))
    {
      return boost::optional<const cfg::setting::Creditcheck&>(cc);
    }
  }
  return boost::optional<const cfg::setting::Creditcheck&>();
}

boost::optional<const cfg::setting::Creditloss&> 
CreditLoss(const User& user, const fs::VirtualPath& path)
{
  for (const auto& cc : cfg::Get().Creditloss())
  {
    if (util::string::WildcardMatch(cc.Path(), path.ToString()) &&
        cc.ACL().Evaluate(user))
    {
      return boost::optional<const cfg::setting::Creditloss&>(cc);
    }
  }
  return boost::optional<const cfg::setting::Creditloss&>();
}

} /* acl namespace */
