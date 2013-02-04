#include "cmd/site/readd.hpp"
#include "acl/usercache.hpp"
#include "acl/allowsitecmd.hpp"
#include "cmd/error.hpp"

namespace cmd { namespace site
{

void READDCommand::Execute()
{
  if (!acl::AllowSiteCmd(client.User(), "readd") &&
      acl::AllowSiteCmd(client.User(), "readdgadmin") &&
      !client.User().HasGadminGID(acl::UserCache::PrimaryGID(acl::UserCache::NameToUID(args[1]))))
  {
    throw cmd::PermissionError();
  }
  
  // needs further checking to ensure
  // gadmins can't exceed their slots
  
  util::Error e = acl::UserCache::Readd(args[1]);
  if (!e)
    control.Reply(ftp::ActionNotOkay, e.Message());
  else
    control.Reply(ftp::CommandOkay, "User " + args[1] + " has been readded.");
  return;
}

} /* site namespace */
} /* cmd namespace */
