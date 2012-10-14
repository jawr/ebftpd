#include "cmd/site/chpass.hpp"
#include "acl/securepass.hpp"
#include "acl/passwdstrength.hpp"
#include "acl/usercache.hpp"

namespace cmd { namespace site
{

void CHPASSCommand::Execute()
{
  static const char* syntax = "Syntax: SITE CHPASS <user> <password>";
  
  if (args.size() != 3)
    control.Reply(ftp::SyntaxError, syntax);
  else
  {
    // checking to ensure gadmins can't change users not in their
    // group, also exclude self from all checking
    
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
      control.MultiReply(ftp::ActionNotOkay, os.str());
      return;
    }
  
    util::Error e = acl::UserCache::SetPassword(args[1], args[2]);
    if (!e)
      control.Reply(ftp::ActionNotOkay, 
                    "Unable to change password: " + e.Message());
    else
      control.Reply(ftp::CommandOkay, "Password changed.");
  }
}

} /* site namespace */
} /* cmd namespace */
