#include <sstream>
#include "cmd/rfc/pass.hpp"
#include "fs/directory.hpp"
#include "db/error.hpp"
#include "ftp/task/task.hpp"
#include "logs/logs.hpp"
#include "cmd/error.hpp"
#include "text/util.hpp"
#include "acl/misc.hpp"
#include "acl/flags.hpp"
#include "fs/path.hpp"

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
      logs::Security("BADPASS", "Login failure by '%1%', bad password", client.User().Name());
      control.Reply(ftp::NotLoggedIn, "Login incorrect.");
      client.SetState(ftp::ClientState::LoggedOut);
    }
    return;
  }
  
  if (!client.PostCheckAddress())
  {
    std::ostringstream identAtAddress;
    identAtAddress << client.Ident() << "@" << client.HostnameAndIP();
    logs::Security("BADIDENTADDRESS", "Refusing login attempt by '%1%' from unknown ident@address: %2%'",
                   client.User().Name(), identAtAddress.str());
    control.Reply(ftp::ServiceUnavailable, "Login not allowed from " + 
          identAtAddress.str() + ".");
    client.SetState(ftp::ClientState::Finished);
    return;
  }
  
  if (client.User().HasFlag(acl::Flag::Deleted))
  {
    logs::Security("LOGINDELETED", "'%1%' attempted to login while deleted", client.User().Name());
    control.Reply(ftp::ServiceUnavailable, "You have been deleted. Goodbye.");
    client.SetState(ftp::ClientState::Finished);
    return;
  }

  if (ftp::Client::IsSiteopOnly() && !client.User().HasFlag(acl::Flag::Siteop))
  {
    control.Reply(ftp::ServiceUnavailable, "Server has been shutdown.");
    return;
  }
  
  util::Error e = fs::ChangeDirectory(client.User(), fs::VirtualPath(client.User().StartUpDir()));
  if (!e) 
  {
    logs::Debug("StartUpDir for", "'%1%' does not exist", client.User().Name());
    util::Error e = fs::ChangeDirectory(client.User(), fs::VirtualPath(client.User().HomeDir()));
    if (!e)
    {
       control.Reply(ftp::ServiceUnavailable, 
       "Unable to change to site root directory: " + e.Message());
       client.SetState(ftp::ClientState::Finished);
       return;
    }
  }
  
  if (client.User().Expired())
  {
    logs::Security("LOGINEXPIRED", "'%1%' attempted to login while expired", client.User().Name());
    control.Reply(ftp::ServiceUnavailable, "Your account has expired.");
    client.SetState(ftp::ClientState::Finished);
    return;
  }
  
  ftp::task::LoginKickUser::Result kickResult;
  if (client.KickLogin())
  {
    logs::Debug("%1% requested a login kick.", client.User().Name());
    std::future<ftp::task::LoginKickUser::Result> future;
    std::make_shared<ftp::task::LoginKickUser>(client.User().ID(), future)->Push();    
    future.wait();
    kickResult = future.get();
  }
  
  try
  {
    client.SetLoggedIn(kickResult.kicked);
  }
  catch (const util::RuntimeError& e)
  {
    control.Reply(ftp::ServiceUnavailable, e.Message());
    client.SetState(ftp::ClientState::Finished);
    return;
  }
  
  std::ostringstream os;
  if (client.KickLogin() && !control.SingleLineReplies())
  {
    os << "Kicked " << kickResult.kicked << " (idle " 
       << kickResult.idleTime << ") of " << kickResult.logins << " login(s).\n";
  }
  
  client.User().SetLoggedIn();

  fs::Path welcomePath(acl::message::Choose<acl::message::Welcome>(client.User()));
  if (!welcomePath.IsEmpty())
  {
    std::string welcome;
    e = text::GenericTemplate(client, welcomePath, welcome);
    if (!e) logs::Error("Failed to display welcome message: %1%", e.Message());
    else os << welcome;
  }
  
  os << "User " << client.User().Name() << " logged in.";

  control.Reply(ftp::UserLoggedIn, os.str());
}

} /* rfc namespace */
} /* cmd namespace */
