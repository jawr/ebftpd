#include "cmd/site/deluser.hpp"
#include "acl/user.hpp"
#include "acl/usercache.hpp"
#include "ftp/listener.hpp"
#include "ftp/task/task.hpp"
#include "ftp/task/types.hpp"

namespace cmd { namespace site
{

cmd::Result DELUSERCommand::Execute()
{
  // needs further flag checking to ensure users with more
  // seniority can't be deleted by those below them
  // and master in config has ultimate seniority
  // lso make so gadmins can only delete their owner users
  acl::User user;
  try
  {
    user = acl::UserCache::User(args[1]);
  }
  catch (const util::RuntimeError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return cmd::Result::Okay;
  }
  
  util::Error e = acl::UserCache::Delete(user.Name());
  if (!e)
    control.Reply(ftp::ActionNotOkay, e.Message());
  else
  {
    ftp::TaskPtr task(new ftp::task::KickUser(user.UID()));
    ftp::Listener::PushTask(task);
    control.Reply(ftp::CommandOkay, "User " + args[1] + " has been deleted.");
  }
  return cmd::Result::Okay;
}

} /* site namespace */
} /* cmd namespace */
