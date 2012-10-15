#include "cmd/site/renuser.hpp"
#include "acl/usercache.hpp"

namespace cmd { namespace site
{

cmd::Result RENUSERCommand::Execute()
{
  util::Error e = acl::UserCache::Rename(args[1], args[2]);
  if (!e)
    control.Reply(ftp::ActionNotOkay, "Unable to rename user: " + e.Message());
  else
    control.Reply(ftp::CommandOkay, "User " + args[1] + " renamed to " + args[2] + ".");
  return cmd::Result::Okay;
}

} /* site namespace */
} /* cmd namespace */
