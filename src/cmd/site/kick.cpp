#include <sstream>
#include "cmd/site/kick.hpp"
#include "acl/user.hpp"
#include "acl/types.hpp"
#include "ftp/task/task.hpp"
#include "ftp/task/types.hpp"
#include "util/error.hpp"
#include "cmd/error.hpp"
#include "logs/logs.hpp"

namespace cmd { namespace site
{

void KICKCommand::Execute()
{
  auto user = acl::User::Load(args[1]);
  if (!user)
  {
    control.Reply(ftp::ActionNotOkay, "User " + args[1] + " doesn't exist.");
    throw cmd::NoPostScriptError();
  }

  if (user->HasFlag(acl::Flag::Siteop) && user->ID() != client.User().ID())
  {
    control.Reply(ftp::ActionNotOkay, "Cannot kick a siteop.");
    throw cmd::NoPostScriptError();
  } 

  boost::unique_future<unsigned> future;
  std::make_shared<ftp::task::KickUser>(user->ID(), future)->Push();

  future.wait();
  unsigned kicked = future.get();
  
  std::ostringstream os;
  os << "Kicked " << future.get() << " of " << args[1] << "'s login(s).";
  control.Reply(ftp::CommandOkay, os.str());
  if (kicked == 0) throw cmd::NoPostScriptError();

  logs::Siteop(client.User().Name(), "KICK", user->Name(), kicked);
}

}
}
