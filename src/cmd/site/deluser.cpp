#include <sstream>
#include "cmd/site/deluser.hpp"
#include "acl/user.hpp"
#include "ftp/task/task.hpp"
#include "ftp/task/types.hpp"
#include "cmd/error.hpp"
#include "acl/misc.hpp"
#include "logs/logs.hpp"
#include "acl/flags.hpp"

namespace cmd { namespace site
{

void DELUSERCommand::Execute()
{
  if (!acl::AllowSiteCmd(client.User(), "deluser") &&
      acl::AllowSiteCmd(client.User(), "delusergadmin") &&
      !client.User().HasGadminGID(acl::NameToPrimaryGID(args[1])))
  {
    throw cmd::PermissionError();
  }

  auto user = acl::User::Load(args[1]);
  if (!user)
  {
    control.Reply(ftp::ActionNotOkay, "User " + args[1] + " doesn't exist.");
    throw cmd::NoPostScriptError();    
  }
  
  user->AddFlag(acl::Flag::Deleted);

  std::future<int> future;
  std::make_shared<ftp::task::KickUser>(user->ID(), future)->Push();
  
  future.wait();
  int kicked = future.get();
  std::ostringstream os;
  os << "User " << args[1] << " has been deleted.";
  if (kicked) os << " (" << kicked << " login(s) kicked)";

  control.Reply(ftp::CommandOkay, os.str());
  logs::Siteop(client.User().Name(), "deleted user '%1%'", user->Name());
}

} /* site namespace */
} /* cmd namespace */
