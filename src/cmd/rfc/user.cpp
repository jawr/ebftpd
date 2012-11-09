#include "cmd/rfc/user.hpp"
#include "acl/usercache.hpp"

namespace cmd { namespace rfc
{

cmd::Result USERCommand::Execute()
{
  bool kickLogin = false;
  if (argStr[0] == '!')
  {
    argStr.erase(0, 1);
    kickLogin = true;
  }
  
  try
  {
      client.SetWaitingPassword(acl::UserCache::User(argStr), kickLogin);
  }
  catch (const util::RuntimeError& e)
  {
    control.Reply(ftp::NotLoggedIn, "User " + argStr + " access denied.");
    return cmd::Result::Okay;
  }
  
  control.Reply(ftp::NeedPassword, "Password required for " + argStr + "."); 
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
