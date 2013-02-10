#include "cmd/site/renuser.hpp"
#include "acl/usercache.hpp"
#include "acl/util.hpp"

namespace cmd { namespace site
{

void RENUSERCommand::Execute()
{
  if (!acl::Validate(acl::ValidationType::Username, args[2]))
  {
    control.Reply(ftp::ActionNotOkay, "Username contains invalid characters");
    return;
  }

  util::Error e = acl::UserCache::Rename(args[1], args[2]);
  if (!e)
    control.Reply(ftp::ActionNotOkay, e.Message());
  else
    control.Reply(ftp::CommandOkay, "User " + args[1] + " renamed to " + args[2] + ".");
}

} /* site namespace */
} /* cmd namespace */
