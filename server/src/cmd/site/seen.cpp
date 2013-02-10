#include <memory>
#include "cmd/site/seen.hpp"
#include "acl/userprofile.hpp"
#include "acl/usercache.hpp"
#include "db/user/userprofile.hpp"
#include "util/error.hpp"

namespace cmd { namespace site
{

void SEENCommand::Execute()
{
  acl::UserProfile profile;
  try
  {
    profile = db::userprofile::Get(acl::UserCache::NameToUID(args[1]));
  }
  catch (const util::RuntimeError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }

  std::ostringstream os;
  if (args[1] == client.User().Name())
    os << "Looking at you right now!";
  else
  if (!profile.LastLogin())
    os << "User " << args[1] << " has never logged in.";
  else
    os << "Last saw " << args[1] << " on " << *profile.LastLogin();
      
  control.Reply(ftp::CommandOkay, os.str());
}

}
}
