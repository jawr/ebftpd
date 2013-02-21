#include <sstream>
#include "cmd/site/addip.hpp"
#include "util/error.hpp"
#include "acl/ipstrength.hpp"
#include "acl/misc.hpp"
#include "cmd/error.hpp"
#include "logs/logs.hpp"

namespace cmd { namespace site
{

void ADDIPCommand::Execute()
{
  if (args[0] == "ADDIP" && !acl::AllowSiteCmd(client.User(), "addip"))
  {
    if (args[1] != client.User().Name() ||
        !acl::AllowSiteCmd(client.User(), "addipown"))
    {
      if (!client.User().HasGadminGID(acl::NameToPrimaryGID(args[1])) ||
          !acl::AllowSiteCmd(client.User(), "addipgadmin"))
      {
        throw cmd::PermissionError();
      }
    }
  }

  auto user = acl::User::Load(args[1]);
  if (!user)
  {
    control.Reply(ftp::ActionNotOkay, "User " + args[1] + " doesn't exist.");
    return;
  }
  
  std::ostringstream os;
  os << "Adding IPs to " << user->Name() << ":";
  
  acl::IPStrength strength;
  std::vector<std::string> deleted;
  for (auto it = args.begin() + 2; it != args.end(); ++it)
  {
    util::Error ok;
    
    if (it->find('@') == std::string::npos)
    {
      os << "\nIP " << *it << " not added. Must be in format ident@address.";
      continue;
    }

    if (!acl::SecureIP(client.User(), *it, strength))
    {
      os << "\nIP " << *it << " not added: Must contain " << strength.String() << ".";
      continue;
    }

    if (!user->AddIPMask(*it, &deleted))
    {
      os << "\nIP " << *it << " not added: Wider matching mask already exists.";
      continue;
    }
      
    os << "\nIP " << *it << " added successfully.";
    logs::Siteop(client.User().Name(), "ADDIP", user->Name(), *it);
    for (const std::string& del : deleted)
    {
      os << "\nAuto-removed unncessary IP " << del << "!";
      logs::Siteop(client.User().Name(), "DELIP", user->Name(), del);
    }
  }

  os << "\nCommand finished.";
  
  control.Reply(ftp::CommandOkay, os.str());
  
} 

// end 
}
}
