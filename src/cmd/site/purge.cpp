#include "cmd/site/purge.hpp"
#include "acl/usercache.hpp"

namespace cmd { namespace site
{

void PURGECommand::Execute()
{
  static const char* syntax = "SITE PURGE <user>";
  if (args.size() != 2)
    control.Reply(ftp::SyntaxError, syntax);
  else
  {
    // option to purge all deleted users?
  
    util::Error e = acl::UserCache::Purge(args[1]);
    if (!e)
      control.Reply(ftp::ActionNotOkay, "Unable to purge user: " + e.Message());
    else
      control.Reply(ftp::CommandOkay, "User " + args[1] + " has been purged.");
  }
}

} /* site namespace */
} /* cmd namespace */
