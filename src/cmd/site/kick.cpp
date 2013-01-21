#include <sstream>
#include "cmd/site/kick.hpp"
#include "acl/user.hpp"
#include "acl/usercache.hpp"
#include "acl/types.hpp"
#include "ftp/task/task.hpp"
#include "ftp/task/types.hpp"
#include "util/error.hpp"
#include "cmd/error.hpp"

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
    throw cmd::NoPostScriptError();
  }

  if (user.CheckFlag(acl::Flag::Siteop) && user.UID() != client.User().UID())
  {
    control.Reply(ftp::ActionNotOkay, "Cannot kick a siteop.");
    throw cmd::NoPostScriptError();
  } 

  boost::unique_future<unsigned> future;
  std::make_shared<ftp::task::KickUser>(user.UID(), future)->Push();

  future.wait();
  unsigned kicked = future.get();
  
  std::ostringstream os;
  os << "Kicked " << future.get() << " of " << args[1] << "'s login(s).";
  control.Reply(ftp::CommandOkay, os.str());
  if (kicked == 0) throw cmd::NoPostScriptError();
}

// end
}
}
