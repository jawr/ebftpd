#include "cmd/site/reload.hpp"
#include "ftp/listener.hpp"
#include "ftp/task/task.hpp"
#include "cfg/config.hpp"

namespace cmd { namespace site
{

cmd::Result RELOADCommand::Execute()
{
  boost::unique_future<bool> future;
  ftp::TaskPtr task(new ftp::task::ReloadConfig(future));
  ftp::Listener::PushTask(task);
  future.wait();
  
  if (!future.get())
  {
    control.PartReply(ftp::ActionNotOkay, "Config failed to reload.");
    control.Reply(ftp::ActionNotOkay, "See SITE LOGS ERROR for further details.");
    return cmd::Result::Okay;
  }
  
  control.Reply(ftp::CommandOkay, "Config reloaded.");
  return cmd::Result::Okay;
}

} /* site namespace */
} /* cmd namespace */
