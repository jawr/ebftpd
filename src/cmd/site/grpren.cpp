#include <cctype>
#include "cmd/site/grpren.hpp"
#include "acl/groupcache.hpp"
#include "acl/util.hpp"

namespace cmd { namespace site
{

void GRPRENCommand::Execute()
{
  if (!acl::Validate(acl::ValidationType::Groupname, args[1]))
  {
    control.Reply(ftp::ActionNotOkay, "Groupname contains invalid characters");
    return;
  }

  util::Error e = acl::GroupCache::Rename(args[1], args[2]);
  if (!e) control.Reply(ftp::ActionNotOkay, e.Message());
  else control.Reply(ftp::CommandOkay, "Group " + args[1] + " renamed to: " + args[2]);
}

} /* site namespace */
} /* cmd namespace */
