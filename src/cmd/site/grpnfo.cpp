#include <cctype>
#include <boost/algorithm/string/trim.hpp>
#include "cmd/site/grpnfo.hpp"
#include "acl/group.hpp"
#include "acl/util.hpp"

namespace cmd { namespace site
{

void GRPNFOCommand::Execute()
{
  argStr.erase(0, args[1].length());
  boost::trim(argStr);
  
  if (!acl::Validate(acl::ValidationType::Tagline, argStr))
  {
    control.Reply(ftp::ActionNotOkay, "Description contains invalid characters");
    return;
  }

  auto group = acl::Group::Load(args[1]);
  if (!group)
  {
    control.Reply(ftp::ActionNotOkay, "Group " + args[1] + " doesn't exist.");
    return;
  }

  group->SetDescription(argStr);
  control.Reply(ftp::CommandOkay, "New description for " + args[1] + ": " + argStr);
}

} /* site namespace */
} /* cmd namespace */
