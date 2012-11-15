#include "cmd/site/renuser.hpp"
#include "acl/usercache.hpp"

namespace cmd { namespace site
{

void RENUSERCommand::Execute()
{
  util::Error e = acl::UserCache::Rename(args[1], args[2]);
  if (!e)
    control.Reply(ftp::ActionNotOkay, e.Message());
  else
    control.Reply(ftp::CommandOkay, "User " + args[1] + " renamed to " + args[2] + ".");
  return;
}

} /* site namespace */
} /* cmd namespace */
