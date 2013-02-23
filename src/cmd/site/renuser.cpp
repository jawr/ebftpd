#include "cmd/site/renuser.hpp"
#include "acl/util.hpp"
#include "logs/logs.hpp"
#include "acl/user.hpp"

namespace cmd { namespace site
{

void RENUSERCommand::Execute()
{
  if (!acl::Validate(acl::ValidationType::Username, args[2]))
  {
    control.Reply(ftp::ActionNotOkay, "Username contains invalid characters");
    return;
  }

  auto user = acl::User::Load(args[1]);
  if (!user)
  {
    control.Reply(ftp::ActionNotOkay, "User " + args[1] + " doesn't exist.");
    return;
  }
  
  if (!user->Rename(args[2]))
  {
    control.Reply(ftp::ActionNotOkay, "User " + args[2] + " already exists.");
    return;
  }
  
  control.Reply(ftp::CommandOkay, "User " + args[1] + " renamed to " + args[2] + ".");
  logs::Siteop(client.User().Name(), "RENUSER", args[1], args[2]);
}

} /* site namespace */
} /* cmd namespace */
