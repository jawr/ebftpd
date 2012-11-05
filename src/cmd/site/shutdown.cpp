#include "cmd/site/shutdown.hpp"
#include "logs/logs.hpp"
#include "ftp/task/task.hpp"
#include "ftp/listener.hpp"

namespace cmd { namespace site
{

cmd::Result SHUTDOWNCommand::Execute()
{
  boost::to_lower(args[1]);
  if (args[1] == "siteop")
  {
    if (!ftp::Client::SetSiteopOnly()) 
      control.Reply(ftp::ActionNotOkay, "Server already shutdown to siteop only.");
    else
    {
      control.Reply(ftp::CommandOkay, "Server shutdown to siteop only.");
      logs::sysop << client.User().Name() << " shutdown the server to siteop only." << logs::endl;
    }
  }
  else
  if (args[1] == "reopen")
  {
    if (!ftp::Client::SetReopen())
      control.Reply(ftp::ActionNotOkay, "Server already open for all users.");
    else
    {
      control.Reply(ftp::CommandOkay, "Server reopened for all users.");
      logs::sysop << client.User().Name() << " reopened the server to all users." << logs::endl;
    }
  }
  else
  if (args[1] == "full")
  {
    control.Reply(ftp::CommandOkay, "Full server shutdown/exit in progress..");
    logs::sysop << client.User().Name() << " initiated a full server shutdown/exit the server." << logs::endl;
    ftp::TaskPtr task(new ftp::task::Exit());
    ftp::Listener::PushTask(task);
  }
  else
    return cmd::Result::SyntaxError;
    
  return cmd::Result::Okay;
}

} /* site namespace */
} /* cmd namespace */
