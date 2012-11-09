#include <sstream>
#include "cmd/rfc/pass.hpp"
#include "fs/directory.hpp"
#include "db/user/userprofile.hpp"
#include "db/exception.hpp"
#include "ftp/task/task.hpp"
#include "ftp/listener.hpp"
#include "logs/logs.hpp"

namespace cmd { namespace rfc
{

cmd::Result PASSCommand::Execute()
{
  if (argStr[0] == '-')
  {
    argStr.erase(0, 1);
    control.SetSingleLineReplies(true);
  }
  
  if (argStr.empty()) return cmd::Result::SyntaxError;

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

  boost::optional<acl::UserProfile> profile;
  try
  {
    profile.reset(db::userprofile::Get(client.User().UID()));
  }
  catch (const db::DBError& e)
  {
    control.Reply(ftp::ServiceUnavailable, e.Message());
    client.SetState(ftp::ClientState::Finished);
    return cmd::Result::Okay;
  }
  
  if (profile->Expired())
  {
    control.Reply(ftp::ServiceUnavailable, "Your account has expired.");
    client.SetState(ftp::ClientState::Finished);
    return cmd::Result::Okay;
  }
  
  ftp::task::LoginKickUser::Result kickResult;
  if (client.KickLogin())
  {
    logs::debug << client.User().Name() << " requested a login kick." << logs::endl;
    boost::unique_future<ftp::task::LoginKickUser::Result> future;
    ftp::TaskPtr task(new ftp::task::LoginKickUser(client.User().UID(), future));
    ftp::Listener::PushTask(task);
    future.wait();
    kickResult = future.get();
  }
  
  try
  {
    client.SetLoggedIn(*profile, kickResult.kicked);
  }
  catch (const util::RuntimeError& e)
  {
    control.Reply(ftp::ServiceUnavailable, e.Message());
    client.SetState(ftp::ClientState::Finished);
    return cmd::Result::Okay;
  }
  
  std::ostringstream os;
  os << "User " << client.User().Name() << " logged in.";
  if (client.KickLogin() && !control.SingleLineReplies())
  {
    os << "\nKicked " << kickResult.kicked << " (idle " 
       << kickResult.idleTime << ") of " << kickResult.logins << " login(s).";
  }
    
  control.MultiReply(ftp::UserLoggedIn, os.str());
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
