#include <cassert>
#include <functional>
#include "acl/allowfxp.hpp"
#include "cfg/get.hpp"

namespace acl
{

bool AllowFxp(const User& user, bool& logging, 
  const std::function<bool(const cfg::setting::AllowFxp&)>& isAllowed)
{
  const cfg::Config& config = cfg::Get();
  for (const auto& af : config.AllowFxp())
  {
    if (af.ACL().Evaluate(user))
    {
      logging = af.Logging();
      return isAllowed(af);
    }
  }
  
  return false;
}

bool AllowFxpSend(const User& user, bool& logging)
{
  return AllowFxp(user, logging, 
      [](const cfg::setting::AllowFxp& af) { return af.Uploads(); });
}

bool AllowFxpReceive(const User& user, bool& logging)
{
  return AllowFxp(user, logging, 
      [](const cfg::setting::AllowFxp& af) { return af.Downloads(); });
}

} /* acl namespace */
