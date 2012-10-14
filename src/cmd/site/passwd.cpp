#include <sstream>
#include "cmd/site/passwd.hpp"
#include "acl/usercache.hpp"
#include "acl/securepass.hpp"
#include "acl/passwdstrength.hpp"

namespace cmd { namespace site
{

void PASSWDCommand::Execute()
{
  static const char* syntax = "Syntax: SITE PASSWD <password>";
  
  if (args.size() != 2)
    control.Reply(ftp::SyntaxError, syntax);
  else
  {
    acl::PasswdStrength strength;
    
    if (!acl::SecurePass(client.User(), args[1], strength))
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
  
    util::Error e = acl::UserCache::SetPassword(client.User().Name(), args[1]);
    if (!e)
      control.Reply(ftp::ActionNotOkay, 
                    "Unable to change password: " + e.Message());
    else
      control.Reply(ftp::CommandOkay, "Password changed.");
  }
}

} /* site namespace */
} /* cmd namespace */
