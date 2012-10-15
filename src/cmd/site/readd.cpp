#include "cmd/site/readd.hpp"
#include "acl/usercache.hpp"

namespace cmd { namespace site
{

cmd::Result READDCommand::Execute()
{
  // needs further checking to ensure
  // gadmins can't exceed their slots
  
  util::Error e = acl::UserCache::Readd(args[1]);
  if (!e)
    control.Reply(ftp::ActionNotOkay, "Unable to readd user: " + e.Message());
  else
    control.Reply(ftp::CommandOkay, "User " + args[1] + " has been readded.");
  return cmd::Result::Okay;
}

} /* site namespace */
} /* cmd namespace */
