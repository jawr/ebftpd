#include <sstream>
#include "cmd/site/grpdel.hpp"
#include "acl/groupcache.hpp"
#include "db/user/user.hpp"
#include "cmd/error.hpp"

namespace cmd { namespace site
{

void GRPDELCommand::Execute()
{
  std::ostringstream acl;
  acl << "=" << args[1];
  std::vector<acl::User> users = db::user::GetByACL(acl.str());

  if (!users.empty())
  {
    control.Reply(ftp::ActionNotOkay, "Unable to delete a group with members.");
    return;
  }

  util::Error e = acl::GroupCache::Delete(args[1]);
  if (!e)
    control.Reply(ftp::ActionNotOkay, e.Message());
  else
    control.Reply(ftp::CommandOkay, "Group " + args[1] + " deleted.");
}

} /* site namespace */
} /* cmd namespace */
