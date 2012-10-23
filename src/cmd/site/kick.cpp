#include "cmd/site/kick.hpp"
#include "acl/user.hpp"
#include "acl/usercache.hpp"
#include "acl/types.hpp"
#include "ftp/listener.hpp"
#include "ftp/task/task.hpp"
#include "ftp/task/types.hpp"
#include "util/error.hpp"

namespace cmd { namespace site
{

cmd::Result KICKCommand::Execute()
{
  acl::User user;
  try
  {
    user = acl::UserCache::User(args[1]);
  }
  catch (const util::RuntimeError& e)
  {
    control.Reply(ftp::ActionNotOkay, "Error: " + e.Message());
    return cmd::Result::Okay;
  }

  if (user.CheckFlag(acl::Flag::FlagSiteop))
  {
    control.Reply(ftp::ActionNotOkay, "Cannot kick a SITEOP.");
    return cmd::Result::Okay;
  } 

  ftp::TaskPtr task(new ftp::task::KickUser(user.UID()));
  ftp::Listener::PushTask(task);

  control.Reply(ftp::CommandOkay, "Kicked " + args[1]);
  return cmd::Result::Okay;
  
}

// end
}
}
