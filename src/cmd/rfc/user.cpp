#include "cmd/rfc/user.hpp"
#include "acl/usercache.hpp"

namespace cmd { namespace rfc
{

cmd::Result USERCommand::Execute()
{
  try
  {
      client.SetWaitingPassword(acl::UserCache::User(argStr));
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
