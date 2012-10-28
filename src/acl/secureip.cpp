#include "acl/secureip.hpp"
#include "acl/ipstrength.hpp"
#include "acl/user.hpp"
#include "cfg/get.hpp"

namespace acl
{

bool SecureIP(const User& user, const std::string& ip, IPStrength& minimum)
{
  IPStrength strength(ip);
  for (auto& si : cfg::Get().SecureIp())
    if (si.ACL().Evaluate(user))
    {
      if (si.Strength().Allowed(strength))
        return true;
      else
      {
        minimum = si.Strength();
        return false;
      }
    }
  return true;
}

} /* acl namespace */
