#include <sstream>
#include "cmd/site/delip.hpp"
#include "acl/usercache.hpp"
#include "acl/ipmaskcache.hpp"
#include "acl/allowsitecmd.hpp"

namespace cmd { namespace site
{

cmd::Result DELIPCommand::Execute()
{
  if (args[1] != client.User().Name() &&
     !acl::AllowSiteCmd(client.User(), "delip"))
  {
    return cmd::Result::Permission;
  }

  acl::User user;
  try
  {
    user = acl::UserCache::User(args[1]);
  }
  catch (const util::RuntimeError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return cmd::Result::Okay;
  }

  std::ostringstream os;
  util::Error ipDeleted;
  for (Args::iterator it = args.begin()+2; it != args.end(); ++it)
  {
    if (it != args.begin()+2) os << "\n";
    ipDeleted = acl::IpMaskCache::Delete(user, *it);
    if (ipDeleted)
      os << "IP (" << *it << ") removed from " << args[1];
    else
      os << ipDeleted.Message();
  }

  control.MultiReply(ftp::CommandOkay, os.str());
  return cmd::Result::Okay;
}

// end
}
}
