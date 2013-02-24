#include "cmd/site/readd.hpp"
#include "acl/misc.hpp"
#include "cmd/error.hpp"
#include "logs/logs.hpp"
#include "acl/flags.hpp"
#include "acl/user.hpp"
#include "acl/group.hpp"

namespace cmd { namespace site
{

void READDCommand::Execute()
{
  bool gadmin = false;
  if (!acl::AllowSiteCmd(client.User(), "readd") &&
      acl::AllowSiteCmd(client.User(), "readdgadmin"))
  {
    if (!client.User().HasGadminGID(acl::NameToPrimaryGID(args[1]))) throw cmd::PermissionError();  
    gadmin = true;
  }
  
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
  
  if (gadmin)
  {
    auto group = acl::Group::Load(user->PrimaryGID());
    if (!group)
    {
      control.Reply(ftp::ActionNotOkay, "Unable to load group " + user->PrimaryGroup() + ".");
      return;
    }
    
    if (group->NumSlotsUsed() >= group->Slots())
    {
      control.Reply(ftp::ActionNotOkay, "Maximum number of slots exceeded for group " + group->Name() + ".");
      return;
    }
  }
  
  user->DelFlag(acl::Flag::Deleted);
  control.Reply(ftp::CommandOkay, "User " + args[1] + " has been readded.");
  logs::Siteop(client.User().Name(), "readded '%1%'", user->Name());
}

} /* site namespace */
} /* cmd namespace */
