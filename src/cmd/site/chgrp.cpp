#include <sstream>
#include "cmd/site/chgrp.hpp"
#include "util/error.hpp"
#include "cmd/error.hpp"
#include "acl/util.hpp"
#include "acl/group.hpp"

namespace cmd { namespace site 
{

void CHGRPCommand::Execute()
{
  auto user = acl::User::Load(args[1]);
  if (!user)
  {
    control.Reply(ftp::ActionNotOkay, "User " + args[1] + " doesn't exist.");
    return;
  }

  std::function<void(const std::vector<acl::GroupID>&)> go;
  std::vector<acl::GroupID> gids;
  auto it = args.begin() + 3;

  if (args[2] == "-")
  {
    go = boost::bind(&acl::User::DelGIDs, user, boost::ref(gids));
  }
  else if (args[2] == "=") 
  {
    go = boost::bind(&acl::User::SetGIDs, user, boost::ref(gids));
  }
  else if (args[2] == "+")
  {
    go = boost::bind(&acl::User::AddGIDs, user, boost::ref(gids));
  }
  else
  {
    go = boost::bind(&acl::User::ToggleGIDs, user, boost::ref(gids));
    --it;
  }
  
  if (it == args.end()) throw cmd::SyntaxError();

  for (; it != args.end(); ++it)
  {
    auto gid = acl::NameToGID(*it);
    if (gid == -1)
    {
      control.Reply(ftp::ActionNotOkay, "Group " + *it + " doesn't exist.");
      return;
    }
    gids.emplace_back(gid);
  }
  
  if (user->PrimaryGID() == -1)
    control.Reply(ftp::CommandOkay, "User " + user->Name() + " not has no groups.");
  else
    control.Reply(ftp::CommandOkay, "User " + user->Name() + " now has groups: " + acl::GroupString(*user));
}

// end
}
}
