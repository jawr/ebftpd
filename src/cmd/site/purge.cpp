#include "cmd/site/purge.hpp"
#include "acl/usercache.hpp"

namespace cmd { namespace site
{

cmd::Result PURGECommand::Execute()
{
  util::Error e = acl::UserCache::Purge(args[1]);
  if (!e)
    control.Reply(ftp::ActionNotOkay, "Unable to purge user: " + e.Message());
  else
    control.Reply(ftp::CommandOkay, "User " + args[1] + " has been purged.");
  return cmd::Result::Okay;
}

} /* site namespace */
} /* cmd namespace */
