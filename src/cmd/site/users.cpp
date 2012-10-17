#include <sstream>
#include <boost/ptr_container/ptr_vector.hpp>
#include "db/acl/acl.hpp"
#include "cmd/site/users.hpp"
#include "util/error.hpp"
#include "acl/types.hpp"
#include "acl/userprofilecache.hpp"

namespace cmd { namespace site
{

cmd::Result USERSCommand::Execute()
{
  std::ostringstream os;
  boost::ptr_vector<acl::User> users;
  util::Error ok = db::GetUsersByACL(users, args[1]);

  if (!ok)
  {
    control.Reply(ftp::ActionNotOkay, ok.Message());
  }
  else if (users.size() > 0)
  {
    os << "Detailed user listing...";
    for (auto& user: users)
    {
      acl::UserProfile profile = acl::UserProfileCache::UserProfile(user.UID());
      //acl::Group group = acl::GroupCache::Group(user.PrimaryGID());
      os << "\nUser: " << user.Name() << " ";
    }
    
    control.MultiReply(ftp::CommandOkay, os.str());
  }
  else
    control.Reply(ftp::ActionNotOkay, "Error, no users found.");

  return cmd::Result::Okay;    
}

// end
}
}
