#include <sstream>
#include "cmd/site/chgrp.hpp"
#include "acl/usercache.hpp"
#include "acl/groupcache.hpp"
#include "util/error.hpp"

namespace cmd { namespace site 
{

cmd::Result CHGRPCommand::Execute()
{
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
  int8_t iterPoint = 2;
  Method method = Method::Default;
  std::ostringstream os;
  util::Error ok;

  if (args[2] == "+") 
  {
    method = Method::Add;
    os << "Adding group(s) to " << args[1] << ":";
  }
  else if (args[2] == "-")
  {
    method = Method::Delete;
    os << "Deleting group(s) from " << args[1] << ":";
  }
  else if (args[2] == "=") 
  {
    ok = acl::UserCache::ResetSecondaryGIDs(args[1]);
    if (!ok)
    {
      control.Reply(ftp::ActionNotOkay, ok.Message());
      return cmd::Result::Okay;
    }
    method = Method::Add;
    os << "Setting group(s) for " << args[1] << ":";
  }

  if (method != Method::Default) ++iterPoint;

  std::vector<std::string>::iterator it = args.begin()+iterPoint;
  for (; it != args.end(); ++it)
  {
    acl::Group group;
    try
    {
      group = acl::GroupCache::Group(*it);
    }
    catch (const util::RuntimeError& e)
    {
      os << "\n" << e.Message();
      continue;
    }
    if (method == Method::Add)
      ok = acl::UserCache::AddSecondaryGID(args[1], group.GID());
    else if (method == Method::Delete)
      ok = acl::UserCache::DelSecondaryGID(args[1], group.GID());
    if (!ok)
      os << "\n" << ok.Message();
    else
      os << "\n" << *it << " okay.";
  }
  os << "\nCommand finished.";
  control.MultiReply(ftp::CommandOkay, os.str());
  return cmd::Result::Okay;
}

// end
}
}
