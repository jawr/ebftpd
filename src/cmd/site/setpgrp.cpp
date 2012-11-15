#include <sstream>
#include "cmd/site/setpgrp.hpp"
#include "acl/user.hpp"
#include "acl/group.hpp"
#include "acl/usercache.hpp"
#include "acl/groupcache.hpp"
#include "util/error.hpp"

namespace cmd { namespace site
{

void SETPGRPCommand::Execute()
{
  acl::Group group;
  try
  {
    group = acl::GroupCache::Group(args[2]);
  }
  catch (const util::RuntimeError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }
  
  acl::GroupID oldGID;
  util::Error ok = acl::UserCache::SetPrimaryGID(args[1], group.GID(), oldGID);
  if (!ok)
    control.Reply(ftp::ActionNotOkay, ok.Message());
  else
  {
    std::ostringstream os;
    if (oldGID != -1) os << "Moved old primary group " << acl::GroupCache::GIDToName(oldGID) << " to secondary.\n";
    os << "Set primary group for " << args[1] << " to: " << group.Name();
    control.Reply(ftp::CommandOkay, os.str());
  }
}

}
}
