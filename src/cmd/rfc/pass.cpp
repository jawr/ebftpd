#include "cmd/rfc/pass.hpp"
#include "fs/directory.hpp"

namespace cmd { namespace rfc
{

cmd::Result PASSCommand::Execute()
{
  if (!client.VerifyPassword(argStr))
  {
    if (client.PasswordAttemptsExceeded())
    {
      control.Reply(ftp::NotLoggedIn,
                  "Password attempts exceeded, disconnecting.");
      client.SetFinished();
    }
    else
    {
      control.Reply(ftp::NotLoggedIn, "Login incorrect.");
      client.SetLoggedOut();
    }
    return cmd::Result::Okay;
  }
  
  if (client.User().Deleted())
  {
    control.Reply(ftp::ServiceUnavailable, "You have been deleted. Goodbye.");
    client.SetFinished();
    return cmd::Result::Okay;
  }
  
  fs::Path rootPath("/");
  util::Error e = fs::ChangeDirectory(client, rootPath);
  if (!e) 
  {
    control.Reply(ftp::ServiceUnavailable, 
      "Unable to change to site root directory: " + e.Message());
    client.SetFinished();
    return cmd::Result::Okay;
  }
  
  control.Reply(ftp::UserLoggedIn, "User " + client.User().Name() + " logged in.");
  client.SetLoggedIn();
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
