#include <sstream>
#include "cmd/site/grpdel.hpp"
#include "cmd/error.hpp"
#include "acl/group.hpp"
#include "logs/logs.hpp"

namespace cmd { namespace site
{

void GRPDELCommand::Execute()
{
  auto group = acl::Group::Load(args[1]);
  if (!group)
  {
    control.Reply(ftp::ActionNotOkay, "Group " + args[1] + " doesn't exist.");
    return;
  }
  
  auto numMembers = group->NumMembers();
  if (numMembers < 0)
  {
    control.Reply(ftp::ActionNotOkay, "Unable to determine number of memmbers.");
    return;
  }
  
  if (numMembers > 0)
  {
    control.Reply(ftp::ActionNotOkay, "Unable to delete group with members.");
    return;
  }

  group->Purge();
  control.Reply(ftp::CommandOkay, "Group " + args[1] + " deleted.");
  logs::Siteop(client.User().Name(), "GRPDEL", args[1]);
}

} /* site namespace */
} /* cmd namespace */
