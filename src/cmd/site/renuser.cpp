#include "cmd/site/renuser.hpp"
#include "acl/usercache.hpp"

namespace cmd { namespace site
{

void RENUSERCommand::Execute()
{
  static const char* syntax = "SITE RENUSER <old username> <new username>";
  if (args.size() != 3)
    control.Reply(ftp::SyntaxError, syntax);
  else
  {
    util::Error e = acl::UserCache::Rename(args[1], args[2]);
    if (!e)
      control.Reply(ftp::ActionNotOkay, "Unable to rename user: " + e.Message());
    else
      control.Reply(ftp::CommandOkay, "User " + args[1] + " renamed to " + args[2] + ".");
  }
}

} /* site namespace */
} /* cmd namespace */
