#include "cmd/site/purge.hpp"
#include "logs/logs.hpp"
#include "acl/flags.hpp"
#include "acl/user.hpp"

namespace cmd { namespace site
{

void PURGECommand::PurgeAll()
{
  auto users = acl::User::GetUsers("*");
  users.erase(std::remove_if(users.begin(), users.end(),
          [](const acl::User& user)
          {
            return !user.HasFlag(acl::Flag::Deleted);
          }), users.end());
  
  if (users.empty())
  {
    control.Reply(ftp::ActionNotOkay, "No deleted users to purge.");
    return;
  }
  
  for (auto& user : users)
  {
    user.Purge();
    logs::Siteop(client.User().Name(), "purged user '%1%'", user.Name());
  }
  
  if (users.size() == 1)
    control.Reply(ftp::CommandOkay, "User " + args[1] + " has been purged.");
  else
    control.Format(ftp::CommandOkay, "Purged %1% users.", users.size());
}

void PURGECommand::Execute()
{
  if (args[1] == "*")
  {
    PurgeAll();
    return;
  }
  
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
