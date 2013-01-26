#include "cmd/site/grpadd.hpp"
#include "acl/groupcache.hpp"
#include "acl/util.hpp"

namespace cmd { namespace site
{

void GRPADDCommand::Execute()
{
  if (!acl::Validate(acl::ValidationType::Groupname, args[1]))
  {
    control.Reply(ftp::ActionNotOkay, "Groupname contains invalid characters");
    return;
  }

  util::Error ok = acl::GroupCache::Create(args[1]);
  if (!ok)
    control.Reply(ftp::ActionNotOkay, ok.Message());
  else
    control.Reply(ftp::CommandOkay, "Group (" + args[1] + ") successfully added.");
  return;  
}

// end
}
}
