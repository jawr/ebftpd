#include <sstream>
#include <memory>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "cmd/site/seen.hpp"
#include "acl/user.hpp"
#include "util/error.hpp"

namespace cmd { namespace site
{

void SEENCommand::Execute()
{
  std::ostringstream os;
  if (args[1] == client.User().Name())
    os << "Looking at you right now!";
  else
  {
    auto user = acl::User::Load(args[1]);
    if (!user)
    {
      control.Reply(ftp::ActionNotOkay, "User " + args[1] + " doesn't exist.");
      return;
    }

    if (!user->LastLogin())
      os << "User " << args[1] << " has never logged in.";
    else
      os << "Last saw " << args[1] << " on " << *user->LastLogin();
  }
      
  control.Reply(ftp::CommandOkay, os.str());
}

}
}
