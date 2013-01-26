#include <cctype>
#include <boost/algorithm/string/trim.hpp>
#include "cmd/site/grpnfo.hpp"
#include "acl/groupcache.hpp"
#include "db/group/groupprofile.hpp"
#include "acl/util.hpp"

namespace cmd { namespace site
{

void GRPNFOCommand::Execute()
{
  argStr.erase(0, args[1].length());
  boost::trim(argStr);
  
  if (!acl::Validate(acl::ValidationType::Tagline, argStr))
  {
    control.Reply(ftp::ActionNotOkay, "Description contains invalid characters");
    return;
  }

  acl::GroupID gid = acl::GroupCache::NameToGID(args[1]);
  if (gid == -1)
  {
    control.Reply(ftp::ActionNotOkay, "Group doesn't exist.");
    return;
  }

  db::groupprofile::SetDescription(gid, argStr);
  control.Reply(ftp::CommandOkay, "New description for " + args[1] + ": " + argStr);
}

} /* site namespace */
} /* cmd namespace */
