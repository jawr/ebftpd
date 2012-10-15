#include <sstream>
#include "cmd/site/addip.hpp"
#include "util/error.hpp"
#include "acl/usercache.hpp"
#include "acl/ipmaskcache.hpp"

namespace cmd { namespace site
{

cmd::Result ADDIPCommand::Execute()
{

  // race condition between the check if user exists and adding ips
  // possibly we should just start adding the ips to the cache and have 
  // the cache throw an exception if user doesn't exist.
  // makes it so it's all in one action -- atomic
  std::ostringstream os;
  acl::User user;
  try
  {
    user = acl::UserCache::User(args[1]);
  }
  catch (const util::RuntimeError& e)
  {
    os << "Error: " << e.Message();
    control.Reply(ftp::ActionNotOkay, os.str());
    return cmd::Result::Okay;
  }

  std::vector<std::string> deleted;
  util::Error ipOkay;
  for (Args::iterator it = args.begin()+2; it != args.end(); ++it)
  {
    if (it != args.begin()+2) os << "\n";
    deleted.clear();
    ipOkay = acl::IpMaskCache::Add(user, *it, deleted);

    if (!ipOkay)
      os << "Error adding " << *it << ": " << ipOkay.Message();
    else
    {
      os << "IP '" << *it << "' successfully added to " << args[1] << ".";

      for (auto& del: deleted)
        os << "Auto-removing useless IP '" << del << "'...";
    }
  }

  control.MultiReply(ftp::CommandOkay, os.str());
  return cmd::Result::Okay;
} 

// end 
}
}
