#include "cmd/site/shutdown.hpp"
#include "logs/logs.hpp"
#include "ftp/task/task.hpp"
#include "cmd/error.hpp"

namespace cmd { namespace site
{

void SHUTDOWNCommand::Execute()
{
  boost::to_lower(args[1]);
  if (args[1] == "siteop")
  {
    if (!ftp::Client::SetSiteopOnly()) 
      control.Reply(ftp::ActionNotOkay, "Server already shutdown to siteop only.");
    else
    {
      control.Reply(ftp::CommandOkay, "Server shutdown to siteop only.");
      logs::Siteop(client.User().Name(), "SITEOPONLY", "Server put into siteop only mode");
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
      logs::Siteop(client.User().Name(), "REOPEN", "Server reopened to all users");
    }
  }
  else
  if (args[1] == "full")
  {
    control.Reply(ftp::ServiceUnavailable, "Full server shutdown/exit in progress..");
    logs::Siteop(client.User().Name(), "SHUTDOWN", "Server has been shutdown.");
    std::make_shared<ftp::task::Exit>()->Push();
  }
  else
    throw cmd::SyntaxError();
}

} /* site namespace */
} /* cmd namespace */
