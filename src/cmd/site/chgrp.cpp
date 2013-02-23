#include <sstream>
#include "cmd/site/chgrp.hpp"
#include "util/error.hpp"
#include "cmd/error.hpp"
#include "acl/util.hpp"
#include "acl/group.hpp"
#include "logs/logs.hpp"
#include "acl/user.hpp"

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

  std::function<void()> go;
  std::vector<acl::GroupID> gids;
  auto it = args.begin() + 3;

  if (args[2] == "-") go = [&]() { user->DelGIDs(gids); };
  else if (args[2] == "=") go = [&]() { user->SetGIDs(gids); };
  else if (args[2] == "+") go = [&]() { user->AddGIDs(gids); };
  else
  {
    go = [&]() { user->ToggleGIDs(gids); };
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
  
  go();
  
  if (user->PrimaryGID() == -1)
    control.Reply(ftp::CommandOkay, "User " + user->Name() + " now has no groups.");
  else
    control.Reply(ftp::CommandOkay, "User " + user->Name() + " now has groups: " + acl::GroupString(*user));
    
  logs::Siteop(client.User().Name(), "CHGRP", user->Name(), acl::GroupString(*user));
}

// end
}
}
