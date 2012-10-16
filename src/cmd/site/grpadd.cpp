#include "cmd/site/grpadd.hpp"
#include "acl/groupcache.hpp"

namespace cmd { namespace site
{

cmd::Result GRPADDCommand::Execute()
{
  util::Error ok = acl::GroupCache::Create(args[1], 
    std::accumulate(args.begin()+2, args.end(), std::string("")));
  if (!ok)
  {
    control.MultiReply(ftp::ActionNotOkay, ok.Message());
    return cmd::Result::Okay;
  }

  control.Reply(ftp::CommandOkay, "Group (" + args[1] + ") successfully added.");
  return cmd::Result::Okay;  
}

// end
}
}
