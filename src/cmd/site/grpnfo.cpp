#include <cctype>
#include <boost/algorithm/string/trim.hpp>
#include "cmd/site/grpnfo.hpp"
#include "acl/groupcache.hpp"
#include "db/group/groupprofile.hpp"

namespace cmd { namespace site
{

const std::string GRPNFOCommand::charsNotAllowed("!%[]");

bool GRPNFOCommand::Valid(const std::string& tagline)
{
  for (char ch : tagline)
    if (!std::isprint(ch) || 
        charsNotAllowed.find(ch) != std::string::npos)
      return false;
  return true;
}

cmd::Result GRPNFOCommand::Execute()
{
  argStr.erase(0, args[1].length());
  boost::trim(argStr);
  
  acl::GroupID gid = acl::GroupCache::NameToGID(args[1]);
  if (gid == -1)
  {
    control.Reply(ftp::ActionNotOkay, "Group doesn't exist.");
    return cmd::Result::Okay;
  }
  
  if (!Valid(argStr))
  {
    control.Reply(ftp::ActionNotOkay, 
                  "Tagline most contain only printable "
                  "characters and none of the following: " +
                  charsNotAllowed);
    return cmd::Result::Okay;
  }

  util::Error ok = db::groupprofile::SetDescription(gid, argStr);
  if (!ok)
    control.Reply(ftp::ActionNotOkay, ok.Message());
  else
    control.Reply(ftp::CommandOkay, "New description for " + args[1] + ": " + argStr);
  
  return cmd::Result::Okay;
}

} /* site namespace */
} /* cmd namespace */
