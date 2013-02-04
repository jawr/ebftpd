#include "cmd/site/chgadmin.hpp"
#include "acl/groupcache.hpp"
#include "acl/usercache.hpp"

namespace cmd { namespace site
{

void CHGADMINCommand::Execute()
{
  acl::GroupID gid = acl::GroupCache::NameToGID(args[2]);
  if (gid == -1)
  {
    control.Format(ftp::ActionNotOkay, "Group %1% doesn't exist.", args[2]);
    return;
  }
  
  bool added;
  auto e = acl::UserCache::ToggleGadminGID(args[1], gid, added);
  if (!e)
    control.Format(ftp::ActionNotOkay, e.Message());
  else
    control.Format(ftp::CommandOkay, "Gadmin flag %1% %2% for %3%.",
                   added ? "added to" : "removed from", args[1], args[2]);
}

} /* site namespace */
} /* cmd namespace */
