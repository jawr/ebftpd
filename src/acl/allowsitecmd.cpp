#include "acl/allowsitecmd.hpp"
#include "cfg/get.hpp"

namespace acl
{

bool AllowSiteCmd(const User& user, const std::string& keyword)
{
	if (keyword.empty()) return true;
  try
  {
    if (cfg::Get().CommandACL(keyword).Evaluate(user)) return true;
  }
  catch (const std::out_of_range&) { }
  return false;
}

} /* acl namespace */
