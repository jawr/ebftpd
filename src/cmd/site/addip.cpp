#include <sstream>
#include "cmd/site/addip.hpp"
#include "util/error.hpp"
#include "acl/usercache.hpp"
#include "acl/ipmaskcache.hpp"
#include "acl/secureip.hpp"
#include "acl/ipstrength.hpp"
#include "acl/allowsitecmd.hpp"
#include "cmd/error.hpp"

namespace cmd { namespace site
{

void ADDIPCommand::Execute()
{
  if (args[1] != client.User().Name() &&
     !acl::AllowSiteCmd(client.User(), "addip"))
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
  os << "Adding IPs to " << user.Name() << ":";
  
  
  acl::IPStrength strength;
  std::vector<std::string> deleted;
  for (auto it = args.begin() + 2; it != args.end(); ++it)
  {
    //if (it != args.begin()+2) os << "\n";
    util::Error ok;
    if (!acl::SecureIP(client.User(), *it, strength))
    {
      std::ostringstream errmsg;
      errmsg << "Must contain " << strength.NumOctets() << " octets";
      if (strength.HasIdent()) errmsg << ", have an ident";
      if (!strength.IsHostname()) errmsg << ", not be a hostname";
      errmsg << ".";
      ok = util::Error::Failure(errmsg.str());
    }
    else
      ok = acl::IpMaskCache::Add(user, *it, deleted);
      
    if (ok)
    {
      os << "\nIP '" << *it << "' added successfully.";
      for (const std::string& del : deleted)
      {
        os << '\n' << "Auto-removed unncessary IP '" << del << "'.";
      }
    }
    else
      os << "\nIP '" << *it << "' not added: " << ok.Message();
  }

  control.Reply(ftp::CommandOkay, os.str());
} 

// end 
}
}
