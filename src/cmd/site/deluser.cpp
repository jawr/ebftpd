#include <sstream>
#include "cmd/site/deluser.hpp"
#include "acl/user.hpp"
#include "acl/usercache.hpp"
#include "ftp/task/task.hpp"
#include "ftp/task/types.hpp"
#include "cmd/error.hpp"

namespace cmd { namespace site
{

void DELUSERCommand::Execute()
{
  // needs further flag checking to ensure users with more
  // seniority can't be deleted by those below them
  // and master in config has ultimate seniority
  // also make so gadmins can only delete their owner users
  acl::User user;
  try
  {
    user = acl::UserCache::User(args[1]);
  }
  catch (const util::RuntimeError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    throw cmd::NoPostScriptError();
  }
  
  util::Error e = acl::UserCache::Delete(user.Name());
  if (!e)
    control.Reply(ftp::ActionNotOkay, e.Message());
  else
  {
    boost::unique_future<unsigned> future;
    std::make_shared<ftp::task::KickUser>(user.UID(), future)->Push();
    
    future.wait();
    unsigned kicked = future.get();
    std::ostringstream os;
    os << "User " << args[1] << " has been deleted.";
    if (kicked) os << " (" << kicked << " login(s) kicked)";

    control.Reply(ftp::CommandOkay, os.str());
  }
}

} /* site namespace */
} /* cmd namespace */
