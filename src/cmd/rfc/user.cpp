#include "cmd/rfc/user.hpp"
#include "acl/usercache.hpp"

namespace cmd { namespace rfc
{

cmd::Result USERCommand::Execute()
{
  acl::User user;
  try
  {
    user = acl::UserCache::User(argStr);
  }
  catch (const util::RuntimeError& e)
  {
    if (argStr == "root")
      user = acl::User("root", 0, "password", "1");
    else
    {
      control.Reply(ftp::NotLoggedIn, "User " + argStr + " access denied.");
      return cmd::Result::Okay;
    }
  }
  
  control.Reply(ftp::NeedPassword, "Password required for " + argStr + "."); 
  client.SetWaitingPassword(user);
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
