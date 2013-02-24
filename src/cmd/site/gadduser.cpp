#include <sstream>
#include "util/string.hpp"
#include "cmd/site/gadduser.hpp"
#include "cmd/site/adduser.hpp"
#include "acl/misc.hpp"
#include "acl/group.hpp"
#include "cmd/error.hpp"

namespace cmd { namespace site
{

void GADDUSERCommand::Execute()
{
  bool gadmin = false;
  if (!acl::AllowSiteCmd(client.User(), "gadduser") &&
       acl::AllowSiteCmd(client.User(), "gaddusergadmin"))
  {
    auto gid = acl::NameToGID(args[1]);
    if (!client.User().HasGadminGID(gid)) throw cmd::PermissionError();
    gadmin = true;
  }

  std::string cpArgStr("GADDUSER ");
  cpArgStr += args[2];
  for (auto it = args.begin() + 3; it != args.end(); ++it)
    cpArgStr += " " + *it;

  std::vector<std::string> cpArgs;
  util::Split(cpArgs, cpArgStr, " ");
  ADDUSERCommand(client, cpArgStr, cpArgs).Execute(args[1], gadmin);
}

// end
}
} 

