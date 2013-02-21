#include <sstream>
#include "cmd/site/setpgrp.hpp"
#include "acl/user.hpp"
#include "acl/group.hpp"
#include "util/error.hpp"
#include "logs/logs.hpp"

namespace cmd { namespace site
{

void SETPGRPCommand::Execute()
{
  auto user = acl::User::Load(args[1]);
  if (!user)
  {
    control.Reply(ftp::ActionNotOkay, "User " + args[1] + " doesn't exist.");
    return;
  }

  auto gid = acl::NameToGID(args[2]);
  if (gid < 0)
  {
    control.Reply(ftp::ActionNotOkay, "Group " + args[2] + " doesn't exist.");
    return;
  }
  
  acl::GroupID oldGID = user->PrimaryGID();
  if (oldGID == gid)
  {
    control.Reply(ftp::ActionNotOkay, "Group " + args[2] + " is already primary for " + args[1] + ".");
    return;
  }
  
  user->SetPrimaryGID(gid);
  std::ostringstream os;
  if (oldGID != -1) os << "Moved old primary group " << acl::GIDToName(oldGID) << " to secondary.\n";
  os << "Set primary group for " << args[1] << " to: " << args[2];
  control.Reply(ftp::CommandOkay, os.str());
  logs::Siteop(client.User().Name(), "SETPGRP", user->Name(), user->PrimaryGroup());
}

}
}
