#include "cmd/site/chpass.hpp"
#include "acl/securepass.hpp"
#include "acl/passwdstrength.hpp"
#include "acl/usercache.hpp"
#include "acl/allowsitecmd.hpp"
#include "cmd/error.hpp"

namespace cmd { namespace site
{

void CHPASSCommand::Execute()
{
  if (args[0] == "CHPASS" && 
      !acl::AllowSiteCmd(client.User(), "chpass") &&
      acl::AllowSiteCmd(client.User(), "chpassgadmin") &&
      !client.User().HasGadminGID(acl::UserCache::PrimaryGID(acl::UserCache::NameToUID(args[1]))))
  {
    throw cmd::PermissionError();
  }
  
  acl::PasswdStrength strength;  
  if (!acl::SecurePass(client.User(), args[2], strength))
  {
    std::ostringstream os;
    os << "Password not strong enough. Must meet the following minimum criteria:\n"
       << strength.UpperCase() << " uppercase, "
       << strength.LowerCase() << " lowercase, "
       << strength.Digits() << " digits, "
       << strength.Others() << " others, "
       << strength.Length() << " length.";
    control.Reply(ftp::ActionNotOkay, os.str());
    return;
  }
  
  util::Error e = acl::UserCache::SetPassword(args[1], args[2]);
  if (!e)
    control.Reply(ftp::ActionNotOkay, e.Message());
  else
    control.Reply(ftp::CommandOkay, "Password changed.");
}

} /* site namespace */
} /* cmd namespace */
