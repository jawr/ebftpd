#include "cmd/site/purge.hpp"
#include "logs/logs.hpp"
#include "acl/flags.hpp"
#include "acl/user.hpp"

namespace cmd { namespace site
{

void PURGECommand::Execute()
{
  auto user = acl::User::Load(args[1]);
  if (!user)
    control.Reply(ftp::ActionNotOkay, "User " + args[1] + " doesn't exist.");
  else
  if (!user->HasFlag(acl::Flag::Deleted))
    control.Reply(ftp::ActionNotOkay, "User " + args[1] + " is not deleted.");
  else
  {
    user->Purge();
    control.Reply(ftp::CommandOkay, "User " + args[1] + " has been purged.");
    logs::Siteop(client.User().Name(), "purged user '%1%'", user->Name());
  }
}

} /* site namespace */
} /* cmd namespace */
