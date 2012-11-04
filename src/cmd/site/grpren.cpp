#include <cctype>
#include "cmd/site/grpren.hpp"
#include "acl/groupcache.hpp"

namespace cmd { namespace site
{

cmd::Result GRPRENCommand::Execute()
{
  util::Error e = acl::GroupCache::Rename(args[1], args[2]);
  if (!e) control.Reply(ftp::ActionNotOkay, e.Message());
  else control.Reply(ftp::CommandOkay, "Group " + args[1] + " renamed to: " + args[2]);
  return cmd::Result::Okay;
}

} /* site namespace */
} /* cmd namespace */
