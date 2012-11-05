#include "cmd/site/shutdown.hpp"
#include "logs/logs.hpp"

namespace cmd { namespace site
{

cmd::Result SHUTDOWNCommand::Execute()
{
  boost::to_lower(args[1]);
  if (args[1] == "siteop")
  {
    ftp::Client::SetSiteopOnly();
    control.Reply(ftp::CommandOkay, "Server shutdown to siteop only.");
    logs::sysop << client.User().Name() << " shutdown the server to siteop only." << logs::endl;
  }
  else
  if (args[1] == "reopen")
  {
    ftp::Client::SetReopen();
    control.Reply(ftp::CommandOkay, "Server reopened for all users.");
    logs::sysop << client.User().Name() << " reopened the server to all users." << logs::endl;
  }
  else
  if (args[1] == "exit")
  {
    control.Reply(ftp::NotImplemented, "Not implemented.");
  }
  else
    return cmd::Result::SyntaxError;
    
  return cmd::Result::Okay;
}

} /* site namespace */
} /* cmd namespace */
