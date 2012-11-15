#include "cmd/site/grpadd.hpp"
#include "acl/groupcache.hpp"

namespace cmd { namespace site
{

void GRPADDCommand::Execute()
{
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
