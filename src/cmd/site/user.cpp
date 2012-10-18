#include <sstream>
#include "cmd/site/user.hpp"
#include "acl/types.hpp"
#include "acl/usercache.hpp"
#include "acl/userprofilecache.hpp"
#include "acl/userprofile.hpp"
#include "util/error.hpp"

namespace cmd { namespace site
{

cmd::Result USERCommand::Execute()
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

  acl::UserProfile profile = acl::UserProfileCache::UserProfile(user.UID());

  std::ostringstream os;

  os << "+=======================================================================+";
  os << "\n| Username: " << user.Name();
  os << "\n| Flags: " << user.Flags();
  os << "\n| Ratio: " << profile.Ratio();

  std::string group = (user.PrimaryGID() == -1) ? "NoGroup" : acl::GroupCache::Group(user.PrimaryGID()).Name();
  os << "\n| Primary Group: " << group;

  auto secondary = user.SecondaryGIDs();
  if (secondary.size() > 0)
  {
    os << "\n| Secondary Groups: ";
    acl::Group group;
    for (auto& gid: user.SecondaryGIDs())
    {
      try
      {
        group = acl::GroupCache::Group(gid);
        os << "\n|\t" << group.Name();
      }
      catch (const util::RuntimeError& e)
      {
        os << "\n| Error: " << e.Message();
      }
    }
  }
    
  os << "\n+=======================================================================+";

  control.MultiReply(ftp::CommandOkay, os.str());
  return cmd::Result::Okay;
}


// end
}
}

