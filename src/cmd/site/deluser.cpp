#include "cmd/site/deluser.hpp"
#include "acl/usercache.hpp"

namespace cmd { namespace site
{

void DELUSERCommand::Execute()
{
  static const char* syntax = "SITE DELUSER <user>";
  if (args.size() != 2)
    control.Reply(ftp::SyntaxError, syntax);
  else
  {
    // needs further flag checking to ensure users with more
    // seniority can't be deleted by those below them
    // and master in config has ultimate seniority
    // lso make so gadmins can only delete their owner users
    
    util::Error e = acl::UserCache::Delete(args[1]);
    if (!e)
      control.Reply(ftp::ActionNotOkay, "Unable to delete user: " + e.Message());
    else
      control.Reply(ftp::CommandOkay, "User " + args[1] + " has been deleted.");
  }
}

} /* site namespace */
} /* cmd namespace */
