#include "cmd/site/grpadd.hpp"
#include "acl/util.hpp"
#include "acl/group.hpp"
#include "logs/logs.hpp"
#include "util/string.hpp"
#include "acl/user.hpp"

namespace cmd { namespace site
{

void GRPADDCommand::Execute()
{
  if (!acl::Validate(acl::ValidationType::Groupname, args[1]))
  {
    control.Reply(ftp::ActionNotOkay, "Groupname contains invalid characters");
    return;
  }
  
  if (args.size() > 2)
  {
    argStr.erase(0, args[1].length());
    util::Trim(argStr);

    if (!acl::Validate(acl::ValidationType::Tagline, argStr))
    {
      control.Reply(ftp::ActionNotOkay, "Description contains invalid characters");
      return;
    }
  }

  auto group = acl::Group::Create(args[1]);
  if (!group)
  {
    control.Reply(ftp::ActionNotOkay, "Group " + args[1] + " alread exists.");
    return;
  }
  
  if (args.size() > 2) group->SetDescription(argStr);  
  control.Reply(ftp::CommandOkay, "Group " + args[1] + " successfully added.");
  logs::Siteop(client.User().Name(), "GRPADD", group->Name(), group->Description());
}

// end
}
}
