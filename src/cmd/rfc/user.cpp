#include "cmd/rfc/user.hpp"
#include "acl/user.hpp"
#include "cmd/error.hpp"
#include "cfg/get.hpp"

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
  
  try
  {
    auto user = acl::User::Load(argStr);
    if (!user)
    {
      control.Reply(ftp::NotLoggedIn, "User " + argStr + " access denied.");
      return;
    }

    if (cfg::Get().TLSControl().Evaluate(*user) && !control.IsTLS())
    {
      control.Reply(ftp::NotLoggedIn, "TLS is enforced on control connections.");
      return;
    }
    
    client.SetWaitingPassword(*user, kickLogin);
  }
  catch (const util::RuntimeError& e)
  {
    control.Reply(ftp::NotLoggedIn, e.Message());
    return;
  }
  
  control.Reply(ftp::NeedPassword, "Password required for " + argStr + "."); 
}

} /* rfc namespace */
} /* cmd namespace */
