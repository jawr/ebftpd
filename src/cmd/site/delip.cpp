#include <sstream>
#include <boost/lexical_cast.hpp>
#include "cmd/site/delip.hpp"
#include "acl/usercache.hpp"
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
  
  std::vector<int> indexes;
  bool all = false;
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
      if (index < 0) throw boost::bad_lexical_cast();
      indexes.emplace_back(index);
    }
    catch (const boost::bad_lexical_cast&)
    {
    }
  }

  if (all)
  {
    std::vector<std::string> deleted;
    util::Error ok = acl::UserCache::DelAllIPMasks(user.Name(), deleted);
    if (!ok)
    {
      control.Reply(ftp::ActionNotOkay, ok.Message());
      return;
    }

    os << "Deleting IPs from " << user.Name();
    for (const std::string& mask : deleted)
    {
      os << "\nIP " << mask << " deleted successfully.";
    }
  }
  else
  {
    os << "Deleting IPs from " << user.Name();
    
    std::sort(indexes.begin(), indexes.end(), std::greater<int>());

    std::string mask;
    for (int index : indexes)
    {
      util::Error ok = acl::UserCache::DelIPMask(user.Name(), index, mask);
      if (!ok)
        os << "\nIP not deleted: " << ok.Message();
      else
        os << "\nIP " << mask << " deleted successfully.";
    }
  }

  os << "\nCommand finished.";
  
  control.Reply(ftp::CommandOkay, os.str());
}

// end
}
}
