#include "cmd/rfc/pass.hpp"
#include "fs/directory.hpp"
#include "db/user/userprofile.hpp"
#include "db/exception.hpp"

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
      client.SetState(ftp::ClientState::Finished);
    }
    else
    {
      control.Reply(ftp::NotLoggedIn, "Login incorrect.");
      client.SetState(ftp::ClientState::LoggedOut);
    }
    return cmd::Result::Okay;
  }
  
  if (client.User().Deleted())
  {
    control.Reply(ftp::ServiceUnavailable, "You have been deleted. Goodbye.");
    client.SetState(ftp::ClientState::Finished);
    return cmd::Result::Okay;
  }

  if (ftp::Client::IsSiteopOnly() && !client.User().CheckFlag(acl::Flag::Siteop))
  {
    control.Reply(ftp::ServiceUnavailable, "Server has been shutdown.");
    return cmd::Result::Okay;
  }
  
  fs::Path rootPath("/");
  util::Error e = fs::ChangeDirectory(client, rootPath);
  if (!e) 
  {
    control.Reply(ftp::ServiceUnavailable, 
      "Unable to change to site root directory: " + e.Message());
    client.SetState(ftp::ClientState::Finished);
    return cmd::Result::Okay;
  }

  try
  {
    client.SetLoggedIn(db::userprofile::Get(client.User().UID()));
  }
  catch (const db::DBError& e)
  {
    control.Reply(ftp::ServiceUnavailable, e.Message());
    client.SetState(ftp::ClientState::Finished);
    return cmd::Result::Okay;
  }
  
  control.Reply(ftp::UserLoggedIn, "User " + client.User().Name() + " logged in.");
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
