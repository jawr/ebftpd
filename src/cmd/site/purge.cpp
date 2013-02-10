#include "cmd/site/purge.hpp"
#include "acl/usercache.hpp"

namespace cmd { namespace site
{

void PURGECommand::Execute()
{
  util::Error e = acl::UserCache::Purge(args[1]);
  if (!e)
    control.Reply(ftp::ActionNotOkay, e.Message());
  else
    control.Reply(ftp::CommandOkay, "User " + args[1] + " has been purged.");
  return;
}

} /* site namespace */
} /* cmd namespace */
