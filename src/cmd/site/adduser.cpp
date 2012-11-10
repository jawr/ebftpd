#include <sstream>
#include "cmd/site/adduser.hpp"
#include "acl/usercache.hpp"
#include "util/error.hpp"
#include "cfg/get.hpp"
#include "cfg/config.hpp"
#include "acl/securepass.hpp"
#include "acl/passwdstrength.hpp"
#include "acl/ipmaskcache.hpp"

namespace cmd { namespace site
{

cmd::Result ADDUSERCommand::Execute()
{
  acl::PasswdStrength strength;
  if (!acl::SecurePass(client.User(), args[2], strength))
  {
    std::ostringstream os;
    os << "Password not strong enough. Must meed the following minimum criteria:\n"
      << strength.UpperCase() << " uppercase, "
      << strength.LowerCase() << " lowercase, "
      << strength.Digits() << " digits, " 
      << strength.Others() << " others, "
      << strength.Length() << " length.";
    control.Reply(ftp::ActionNotOkay, os.str());
    return cmd::Result::Okay;
  }
  
  const cfg::Config& cfg = cfg::Get();

  util::Error ok = acl::UserCache::Create(args[1], args[2], 
    cfg.DefaultFlags(), client.User().UID());

  if (!ok)
  {
    control.Reply(ftp::ActionNotOkay, ok.Message());
    return cmd::Result::Okay;
  }

  std::ostringstream os;
  os << "Added user " << args[1] << ".";

  if (args.size() > 3)
  {
    const acl::User user = acl::UserCache::User(args[1]);
    std::vector<std::string> deleted;
    util::Error ipOkay;
    for (Args::iterator it = args.begin()+3; it != args.end(); ++it)
    {
      ipOkay = acl::IpMaskCache::Add(user, *it, deleted);
      if (!ipOkay)
      {
        os << "\n\tError adding " << *it << ": " << ipOkay.Message();
        continue;
      }

      os << "\nIP '" << *it << "' successfully added to " << args[1] << ".";
    }
  }

  control.Reply(ftp::CommandOkay, os.str());
  return cmd::Result::Okay;
}

// end
}
} 
