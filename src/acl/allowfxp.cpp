#include <cassert>
#include "acl/allowfxp.hpp"
#include "cfg/get.hpp"

namespace acl
{

bool AllowFxp(ftp::TransferType transferType,
              const User& user, bool& logging)
{
  assert(transferType != ftp::TransferType::List);

  const cfg::Config& config = cfg::Get();
  for (const auto& af : config.AllowFxp())
  {
    if (af.ACL().Evaluate(user))
    {
      logging = af.Logging();
      if (transferType == ftp::TransferType::Download)
        return af.Downloads();
      else
        return af.Uploads();
    }
  }
  
  return false;
}

} /* acl namespace */
