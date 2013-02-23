#include <algorithm>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include "cmd/site/delip.hpp"
#include "acl/misc.hpp"
#include "cmd/error.hpp"
#include "logs/logs.hpp"
#include "acl/user.hpp"

namespace cmd { namespace site
{

void DELIPCommand::Execute()
{
  if (!acl::AllowSiteCmd(client.User(), "delip"))
  {
    if (args[1] != client.User().Name() ||
        !acl::AllowSiteCmd(client.User(), "delipown"))
    {
      if (!client.User().HasGadminGID(acl::NameToPrimaryGID(args[1])) ||
          !acl::AllowSiteCmd(client.User(), "delipgadmin"))
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
  
  std::vector<int> indexes;
  bool all = false;
  const auto& masks = user->IPMasks();
  for (auto it = args.begin() + 2; it != args.end(); ++it)
  {
    if (*it == "*")
    {
      all = true;
      indexes.clear();
      break;
    }
    
    try
    {
      int index = boost::lexical_cast<int>(*it);
      if (index < 0 || index > static_cast<ssize_t>(masks.size()) - 1)
      {
        control.Reply(ftp::ActionNotOkay, "IP mask index out of range.");
        return;
      }
      
      indexes.emplace_back(index);
    }
    catch (const boost::bad_lexical_cast&)
    {
      auto it2 = std::find(masks.begin(), masks.end(), *it);
      if (it2 == user->IPMasks().end())
      {
        control.Reply(ftp::ActionNotOkay, "No IP mask matching " + *it + " exists for " + args[1] + ".");
        return;
      }
      indexes.emplace_back(std::distance(masks.begin(), it2));
    }
  }

  os << "Deleting IPs from " << user->Name();
  if (all)
  {
    std::vector<std::string> deleted = user->IPMasks();
    user->ClearIPMasks();
    for (const std::string& mask : deleted)
    {
      os << "\nIP " << mask << " deleted successfully.";
      logs::Siteop(client.User().Name(), "deleted ip '%2%' from '%1%'", user->Name(), mask);
    }
  }
  else
  {
    std::sort(indexes.begin(), indexes.end(), std::greater<int>());
    std::string mask;
    for (int index : indexes)
    {
      std::string mask = user->DelIPMask(index);
      os << "\nIP " << mask << " deleted successfully.";
      logs::Siteop(client.User().Name(), "deleted ip '%2%' from '%1%'", user->Name(), mask);
    }
  }

  os << "\nCommand finished.";
  
  control.Reply(ftp::CommandOkay, os.str());
}

// end
}
}
