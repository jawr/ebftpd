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
  acl::User user;
  acl::UserProfile profile;
  try
  {
    user = acl::UserCache::User(args[1]);
    profile = db::userprofile::Get(user.UID());
  }
  catch (const util::RuntimeError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }

  std::ostringstream os;
  if (!profile.LastLogin())
    os << "User " << args[1] << " has never logged in.";
  else
    os << "Last saw " << args[1] << " on " << *profile.LastLogin();
      
  control.Reply(ftp::CommandOkay, os.str());
}

}
}
