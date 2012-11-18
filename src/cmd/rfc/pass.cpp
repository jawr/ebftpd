#include <sstream>
#include "cmd/rfc/pass.hpp"
#include "fs/directory.hpp"
#include "db/user/userprofile.hpp"
#include "db/exception.hpp"
#include "ftp/task/task.hpp"
#include "ftp/listener.hpp"
#include "logs/logs.hpp"
#include "cmd/error.hpp"

namespace cmd { namespace rfc
{

void PASSCommand::Execute()
{
  if (argStr[0] == '-')
  {
    argStr.erase(0, 1);
    control.SetSingleLineReplies(true);
  }
  
  if (argStr.empty()) throw cmd::SyntaxError();

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
    return;
  }
  
  if (client.User().Deleted())
  {
    control.Reply(ftp::ServiceUnavailable, "You have been deleted. Goodbye.");
    client.SetState(ftp::ClientState::Finished);
    return;
  }

  if (ftp::Client::IsSiteopOnly() && !client.User().CheckFlag(acl::Flag::Siteop))
  {
    control.Reply(ftp::ServiceUnavailable, "Server has been shutdown.");
    return;
  }
  
  fs::VirtualPath rootPath("/");
  util::Error e = fs::ChangeDirectory(client, rootPath);
  if (!e) 
  {
    control.Reply(ftp::ServiceUnavailable, 
      "Unable to change to site root directory: " + e.Message());
    client.SetState(ftp::ClientState::Finished);
    return;
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
    return;
  }
  
  if (profile->Expired())
  {
    control.Reply(ftp::ServiceUnavailable, "Your account has expired.");
    client.SetState(ftp::ClientState::Finished);
    return;
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
    return;
  }
  
  std::ostringstream os;
  os << "User " << client.User().Name() << " logged in.";
  if (client.KickLogin() && !control.SingleLineReplies())
  {
    os << "\nKicked " << kickResult.kicked << " (idle " 
       << kickResult.idleTime << ") of " << kickResult.logins << " login(s).";
  }
  
  db::userprofile::Login(client.User().UID());
      
  control.Reply(ftp::UserLoggedIn, os.str());
  return;
}

} /* rfc namespace */
} /* cmd namespace */
