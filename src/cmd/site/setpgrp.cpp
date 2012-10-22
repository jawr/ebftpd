#include "cmd/site/setpgrp.hpp"
#include "acl/user.hpp"
#include "acl/group.hpp"
#include "acl/usercache.hpp"
#include "acl/groupcache.hpp"
#include "util/error.hpp"

namespace cmd { namespace site
{

cmd::Result SETPGRPCommand::Execute()
{
  acl::Group group;
  try
  {
    group = acl::GroupCache::Group(args[2]);
  }
  catch (const util::RuntimeError& e)
  {
    control.Reply(ftp::ActionNotOkay, "Error: " + e.Message());
    return cmd::Result::Okay;
  }

  util::Error ok = acl::UserCache::SetPrimaryGID(args[1], group.GID());
  if (!ok)
    control.Reply(ftp::ActionNotOkay, "Error: " + ok.Message());
  else
    control.Reply(ftp::CommandOkay, "Set primary group for " + args[1] + " to: " + group.Name());
  return cmd::Result::Okay;
}

}
}
