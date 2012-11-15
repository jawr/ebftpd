#include <sstream>
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

void KICKCommand::Execute()
{
  acl::User user;
  try
  {
    user = acl::UserCache::User(args[1]);
  }
  catch (const util::RuntimeError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }

  if (user.CheckFlag(acl::Flag::Siteop))
  {
    control.Reply(ftp::ActionNotOkay, "Cannot kick a siteop.");
    return;
  } 

  boost::unique_future<unsigned> future;
  ftp::TaskPtr task(new ftp::task::KickUser(user.UID(), future));
  ftp::Listener::PushTask(task);

  future.wait();
  
  std::ostringstream os;
  os << "Kicked " << future.get() << " of " << args[1] << "'s login(s).";
  control.Reply(ftp::CommandOkay, os.str());
  return;
  
}

// end
}
}
