#include <sstream>
#include "cmd/site/grpdel.hpp"
#include "acl/groupcache.hpp"
#include "db/user/user.hpp"
#include "main.hpp"

namespace cmd { namespace site
{

void GRPDELCommand::Execute()
{
  std::ostringstream acl;
  acl << "=" << args[1];
  boost::ptr_vector<acl::User> users;
  
  util::Error e = db::user::UsersByACL(users, acl.str());
  if (!e) 
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }

  if (!users.empty())
  {
    control.Reply(ftp::ActionNotOkay, "Unable to delete a group with members.");
    return;
  }

  e = acl::GroupCache::Delete(args[1]);
  if (!e)
    control.Reply(ftp::ActionNotOkay, e.Message());
  else
    control.Reply(ftp::CommandOkay, "Group " + args[1] + " deleted.");
}

} /* site namespace */
} /* cmd namespace */
