#include "cmd/rfc/user.hpp"
#include "acl/usercache.hpp"

namespace cmd { namespace rfc
{

void USERCommand::Execute()
{
  if (argStr.empty())
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }

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
      return;
    }
  }

/*  if (argStr != client.User().Name())
  {
    control.Reply(ftp::NotLoggedIn, "User " + argStr + " access denied.");
    return;
  }*/
  
  control.Reply(ftp::NeedPassword, "Password required for " + argStr + "."); 
  client.SetWaitingPassword(user);
}

} /* rfc namespace */
} /* cmd namespace */
