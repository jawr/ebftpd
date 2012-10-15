#include <vector>
#include <boost/algorithm/string/split.hpp>
#include "acl/allowsitecmd.hpp"
#include "cfg/get.hpp"

namespace acl
{

bool AllowSiteCmd(const User& user, const std::string& keyword)
{
  std::vector<std::string> toks;
  boost::split(toks, keyword, boost::is_any_of("|"));
	if (toks.empty()) return true;
  for (auto& tok : toks)
  {
    try
    {
      if (cfg::Get().CommandACL(tok).Evaluate(user)) return true;
    }
    catch (const std::out_of_range&) { }
  }
  return false;
}

} /* acl namespace */
