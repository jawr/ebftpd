#include "cmd/site/chgadmin.hpp"
#include "acl/user.hpp"
#include "acl/group.hpp"

namespace cmd { namespace site
{

void CHGADMINCommand::Execute()
{
  acl::GroupID gid = acl::NameToGID(args[2]);
  if (gid == -1)
  {
    control.Format(ftp::ActionNotOkay, "Group %1% doesn't exist.", args[2]);
    return;
  }
  
  auto user = acl::User::Load(args[1]);
  if (!user)
  {
    control.Format(ftp::ActionNotOkay, "User %1% doesn't exist.", args[1]);
    return;
  }

  if (!user->HasGID(gid))
  {
    control.Format(ftp::ActionNotOkay, "User %1% is not a member of %2%.", args[1], args[2]);
    return;
  }
  
  bool added = user->ToggleGadminGID(gid);
  control.Format(ftp::CommandOkay, "Gadmin flag %1% %2% for %3%.",
                 added ? "added to" : "removed from", args[1], args[2]);
}

} /* site namespace */
} /* cmd namespace */
