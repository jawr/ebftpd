#include <sstream>
#include "cmd/site/chgrp.hpp"
#include "acl/usercache.hpp"
#include "acl/groupcache.hpp"
#include "util/error.hpp"
#include "cmd/error.hpp"

namespace cmd { namespace site 
{

void CHGRPCommand::Execute()
{
  std::vector<std::string>::size_type iterPoint = 2;
  Method method = Method::Default;
  std::ostringstream os;
  util::Error ok;

  if (args[2] == "+")
  {
    method = Method::Add;
    os << "Adding group(s) to " << args[1] << ":";
  }
  else
  if (args[2] == "-")
  {
    method = Method::Delete;
    os << "Deleting group(s) from " << args[1] << ":";
  }
  else if (args[2] == "=") 
  {
    ok = acl::UserCache::ResetGIDs(args[1]);
    if (!ok)
    {
      control.Reply(ftp::ActionNotOkay, ok.Message());
      return;
    }
    method = Method::Add;
    os << "Setting group(s) for " << args[1] << ":";
  }

  if (method != Method::Default) ++iterPoint;

  std::vector<std::string>::iterator it = args.begin() + iterPoint;
  if (it == args.end()) throw cmd::SyntaxError();
  
  std::string action(method == Method::Add ? "added" : "deleted");
  
  for (; it != args.end(); ++it)
  {
    acl::GroupID gid = acl::GroupCache::NameToGID(*it);
    if (gid == -1)
    {
      os << "\nGroup " << *it << " doesn't exist.";
      continue;
    }
    
    if (method == Method::Add || method == Method::Default)
      ok = acl::UserCache::AddGID(args[1], gid);
    else if (method == Method::Delete)
      ok = acl::UserCache::DelGID(args[1], gid);
    if (!ok)
      os << "\nGroup " << *it << " not " << action << ": " << ok.Message();
    else
      os << "\nGroup " << *it << " " << action << " successfully.";
  }
  
  os << "\nCommand finished.";
  
  control.Reply(ftp::CommandOkay, os.str());
}

// end
}
}
