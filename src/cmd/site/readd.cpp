#include "cmd/site/readd.hpp"
#include "acl/usercache.hpp"

namespace cmd { namespace site
{

void READDCommand::Execute()
{
  static const char* syntax = "SITE READD <user>";
  if (args.size() != 2)
    control.Reply(ftp::SyntaxError, syntax);
  else
  {
    // needs further checking to ensure
    // gadmins can't exceed their slots
    
    util::Error e = acl::UserCache::Readd(args[1]);
    if (!e)
      control.Reply(ftp::ActionNotOkay, "Unable to readd user: " + e.Message());
    else
      control.Reply(ftp::CommandOkay, "User " + args[1] + " has been readded.");
  }
}

} /* site namespace */
} /* cmd namespace */
