#include <sstream>
#include <boost/ptr_container/ptr_vector.hpp>
#include "cmd/site/users.hpp"
#include "util/error.hpp"
#include "acl/types.hpp"
#include "db/user/user.hpp"
#include "db/user/userprofile.hpp"
#include "logs/logs.hpp"

namespace cmd { namespace site
{

cmd::Result USERSCommand::Execute()
{
  std::ostringstream os;
  std::vector<acl::User> users;
  
  if (args.size() == 2)
    users = db::user::GetByACL(args[1]);
  else
    users = db::user::GetAll();

  if (users.size() > 0)
  {
    os << "Detailed user listing...";
    for (auto& user: users)
    {
      acl::UserProfile profile = db::userprofile::Get(user.UID());
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
