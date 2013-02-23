#include "cmd/site/chpass.hpp"
#include "acl/misc.hpp"
#include "acl/passwdstrength.hpp"
#include "acl/misc.hpp"
#include "cmd/error.hpp"
#include "logs/logs.hpp"
#include "acl/user.hpp"

namespace cmd { namespace site
{

void CHPASSCommand::Execute()
{
  if (args[0] == "CHPASS" && 
      !acl::AllowSiteCmd(client.User(), "chpass") &&
      acl::AllowSiteCmd(client.User(), "chpassgadmin") &&
      !client.User().HasGadminGID(acl::NameToPrimaryGID(args[1])))
  {
    throw cmd::PermissionError();
  }
  
  acl::PasswdStrength strength;  
  if (!acl::SecurePass(client.User(), args[2], strength))
  {
    std::ostringstream os;
    os << "Password not strong enough. Must meet the following minimum criteria:\n"
       << strength.String() << ".";
    control.Reply(ftp::ActionNotOkay, os.str());
    return;
  }

  auto user = acl::User::Load(args[1]);
  if (!user)
  {
    control.Reply(ftp::ActionNotOkay, "User " + args[1] + " doesn't exist.");
    return;
  }
  
  user->SetPassword(args[2]);
  control.Reply(ftp::CommandOkay, "Password changed.");
  logs::Siteop(client.User().Name(), "CHPASS", user->Name());
}

} /* site namespace */
} /* cmd namespace */
