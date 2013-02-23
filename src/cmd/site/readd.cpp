#include "cmd/site/readd.hpp"
#include "acl/misc.hpp"
#include "cmd/error.hpp"
#include "logs/logs.hpp"
#include "acl/flags.hpp"
#include "acl/user.hpp"

namespace cmd { namespace site
{

void READDCommand::Execute()
{
  if (!acl::AllowSiteCmd(client.User(), "readd") &&
      acl::AllowSiteCmd(client.User(), "readdgadmin") &&
      !client.User().HasGadminGID(acl::NameToPrimaryGID(args[1])))
  {
    throw cmd::PermissionError();
  }
  
  // needs further checking to ensure
  // gadmins can't exceed their slots
  
  auto user = acl::User::Load(args[1]);
  if (!user)
  {
    control.Reply(ftp::ActionNotOkay, "User " + args[1] + " doesn't exist.");
    return;
  }
  
  if (!user->HasFlag(acl::Flag::Deleted))
  {
    control.Reply(ftp::ActionNotOkay, "User " + args[1] + " is not deleted.");
    return;
  }
  
  user->DelFlag(acl::Flag::Deleted);
  control.Reply(ftp::CommandOkay, "User " + args[1] + " has been readded.");
  logs::Siteop(client.User().Name(), "readded '%1%'", user->Name());
}

} /* site namespace */
} /* cmd namespace */
