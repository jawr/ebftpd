#include <boost/algorithm/string/trim.hpp>
#include "cmd/site/grpadd.hpp"
#include "acl/groupcache.hpp"
#include "acl/util.hpp"
#include "db/group/groupprofile.hpp"

namespace cmd { namespace site
{

void GRPADDCommand::Execute()
{
  std::string group = args[1];
  if (!acl::Validate(acl::ValidationType::Groupname, args[1]))
  {
    control.Reply(ftp::ActionNotOkay, "Groupname contains invalid characters");
    return;
  }
  
  if (args.size() > 2)
  {
    argStr.erase(0, args[1].length());
    boost::trim(argStr);

    if (!acl::Validate(acl::ValidationType::Tagline, argStr))
    {
      control.Reply(ftp::ActionNotOkay, "Description contains invalid characters");
      return;
    }
  }
  
  util::Error ok = acl::GroupCache::Create(args[1]);
  if (!ok)
    control.Reply(ftp::ActionNotOkay, ok.Message());
  else
  {
    if (args.size() > 2) 
    {
      auto gid = acl::GroupCache::NameToGID(args[1]);
      db::groupprofile::SetDescription(gid, argStr);
    }
    control.Reply(ftp::CommandOkay, "Group " + args[1] + " successfully added.");
  }
}

// end
}
}
