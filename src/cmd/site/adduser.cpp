#include <sstream>
#include "cmd/site/adduser.hpp"
#include "acl/usercache.hpp"
#include "util/error.hpp"
#include "cfg/get.hpp"
#include "cfg/config.hpp"

namespace cmd { namespace site
{

void ADDUSERCommand::Execute()
{
  static const char* syntax = "Syntax: SITE ADDUSER <user> <password> [<ident@ip#1> ... <ident@ip#n>]";

  if (args.size() < 3)
  {
    control.Reply(ftp::SyntaxError, syntax);
    return;
  }

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
    control.MultiReply(ftp::ActionNotOkay, os.str());
    return;
  }
  
  const cfg::Config& cfg = cfg::Get();

  util::Error ok = acl::UserCache::Create(args[1], args[2], 
    cfg.DefaultFlags());

  if (!ok)
  {
    control.MultiReply(ftp::ActionNotOkay, ok.Message());
    return;
  }

  std::ostringstream os;
  os << "Added user " << args[1] << ".";

  if (args.size() > 3)
  {
    const acl::User user = acl::UserCache::User(args[1]);
    std::vector<std::string> deleted;
    util::Error ipOkay;
    for (Args::iterator it = args.begin()+3; it != args.end();
      ++it)
    {
      deleted.clear();
      ipOkay = acl::IpMaskCache::Add(user, (*it), deleted);

      if (!ipOkay)
      {
        os << "\n\tError adding " << (*it) << ": " << ipOkay.Message();
        continue;
      }

      os << "\n\tAdded IP Mask: " << (*it);
      if (deleted.size() > 0)
      {
        os << "\n\tAdding broader mask, deleted (" << deleted.size() << "):";
        for (auto& del: deleted)
          os << "\n\t" << del; 
      }
    }
  }

  control.MultiReply(ftp::CommandOkay, os.str());
    return;
}

// end
}
} 
