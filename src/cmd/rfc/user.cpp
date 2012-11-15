#include "cmd/rfc/user.hpp"
#include "acl/usercache.hpp"
#include "cmd/error.hpp"

namespace cmd { namespace rfc
{

void USERCommand::Execute()
{
  bool kickLogin = false;
  if (argStr[0] == '!')
  {
    argStr.erase(0, 1);
    kickLogin = true;
  }
  
  if (argStr.empty()) throw cmd::SyntaxError();
  
  try
  {
      client.SetWaitingPassword(acl::UserCache::User(argStr), kickLogin);
  }
  catch (const util::RuntimeError& e)
  {
    control.Reply(ftp::NotLoggedIn, "User " + argStr + " access denied.");
    return;
  }
  
  control.Reply(ftp::NeedPassword, "Password required for " + argStr + "."); 
  return;
}

} /* rfc namespace */
} /* cmd namespace */
