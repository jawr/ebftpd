#include <memory>
#include "cmd/site/seen.hpp"
#include "acl/userprofile.hpp"
#include "acl/usercache.hpp"
#include "db/user/userprofile.hpp"
#include "util/error.hpp"

namespace cmd { namespace site
{

cmd::Result SEENCommand::Execute()
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
    return cmd::Result::Okay;
  }

  if (profile.LastLogin().empty())
    control.Reply(ftp::CommandOkay, "No seen record for " + args[1] + ".");
  else
    control.Reply(ftp::CommandOkay, 
      "Last saw " + args[1] + " on " + profile.LastLogin());
  return cmd::Result::Okay;
}

}
}
