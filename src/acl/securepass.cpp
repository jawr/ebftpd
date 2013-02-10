#include "acl/securepass.hpp"
#include "acl/passwdstrength.hpp"
#include "acl/user.hpp"
#include "cfg/get.hpp"

namespace acl
{

bool SecurePass(const User& user, const std::string& password , PasswdStrength& minimum)
{
  PasswdStrength strength(password);
  for (auto& sp : cfg::Get().SecurePass())
    if (sp.ACL().Evaluate(user))
    {
      if (sp.Strength().Allowed(strength))
        return true;
      else
      {
        minimum = sp.Strength();
        return false;
      }
    }
  return true;
}

} /* acl namespace */
