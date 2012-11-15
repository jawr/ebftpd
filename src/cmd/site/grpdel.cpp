#include <sstream>
#include "cmd/site/grpdel.hpp"
#include "acl/groupcache.hpp"
#include "db/user/user.hpp"
#include "main.hpp"

namespace cmd { namespace site
{

cmd::Result GRPDELCommand::Execute()
{
  std::ostringstream acl;
  acl << "=" << args[1];
  std::vector<acl::User> users = db::user::GetByACL(acl.str());

  if (!users.empty())
  {
    control.Reply(ftp::ActionNotOkay, "Unable to delete a group with members.");
    return cmd::Result::Okay;
  }

  util::Error e = acl::GroupCache::Delete(args[1]);
  if (!e)
    control.Reply(ftp::ActionNotOkay, e.Message());
  else
    control.Reply(ftp::CommandOkay, "Group " + args[1] + " deleted.");

  return cmd::Result::Okay;
}

} /* site namespace */
} /* cmd namespace */
