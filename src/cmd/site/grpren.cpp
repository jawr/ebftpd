#include <cctype>
#include "cmd/site/grpren.hpp"
#include "acl/util.hpp"
#include "acl/group.hpp"
#include "logs/logs.hpp"
#include "acl/user.hpp"

namespace cmd { namespace site
{

void GRPRENCommand::Execute()
{
  if (!acl::Validate(acl::ValidationType::Groupname, args[1]))
  {
    control.Reply(ftp::ActionNotOkay, "Groupname contains invalid characters");
    return;
  }

  auto group = acl::Group::Load(args[1]);
  if (!group)
  {
    control.Reply(ftp::ActionNotOkay, "Group " + args[1] + " doesn't exist.");
    return;
  }
  
  if (!group->Rename(args[2]))
  {
    control.Reply(ftp::ActionNotOkay, "Group " + args[2] + " already exists.");
    return;
  }
  
  control.Reply(ftp::CommandOkay, "Group " + args[1] + " renamed to: " + args[2]);
  logs::Siteop(client.User().Name(), "renamed group '%1% to '%2%'", args[1], args[2]);
}

} /* site namespace */
} /* cmd namespace */
