#include <sstream>
#include <boost/lexical_cast.hpp>
#include "cmd/site/delip.hpp"
#include "acl/usercache.hpp"
#include "acl/ipmaskcache.hpp"
#include "acl/allowsitecmd.hpp"
#include "cmd/error.hpp"

namespace cmd { namespace site
{

void DELIPCommand::Execute()
{
  if (args[1] != client.User().Name() &&
     !acl::AllowSiteCmd(client.User(), "delip"))
  {
    throw cmd::PermissionError();
  }

  acl::User user;
  try
  {
    user = acl::UserCache::User(args[1]);
  }
  catch (const util::RuntimeError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }

  std::ostringstream os;
  util::Error ipDeleted;

  // check if we are deleting all (*)
  if (args.size() == 3 && args.at(2) == "*")
  {
    args.erase(args.begin()+2);
    std::vector<std::string> masks;
    ipDeleted = acl::IpMaskCache::List(user, masks);
    if (ipDeleted)
      for (auto& i : masks)
        args.push_back(i);
    else
      os << "Error: " << ipDeleted.Message() << "\n";
  }

  for (Args::iterator it = args.begin()+2; it != args.end(); ++it)
  {
    if (it != args.begin()+2) os << "\n";
    std::string mask = *it;

    try
    {
      int idx = boost::lexical_cast<int>(*it);
      ipDeleted = acl::IpMaskCache::Delete(user, idx);
      mask = "IP" + mask;
    }
    catch (const boost::bad_lexical_cast&)
    {
      ipDeleted = acl::IpMaskCache::Delete(user, *it);
    }
    if (ipDeleted)
      os << "IP (" << mask << ") removed from " << args[1];
    else
      os << ipDeleted.Message();
  }

  control.Reply(ftp::CommandOkay, os.str());
}

// end
}
}
