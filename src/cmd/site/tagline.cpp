#include <cctype>
#include "cmd/site/tagline.hpp"
#include "acl/usercache.hpp"
#include "db/user/userprofile.hpp"

namespace cmd { namespace site
{

const std::string TAGLINECommand::charsNotAllowed("!%[]");

bool TAGLINECommand::Valid(const std::string& tagline)
{
  for (char ch : tagline)
    if (!std::isprint(ch) || 
        charsNotAllowed.find(ch) != std::string::npos)
      return false;
  return true;
}

cmd::Result TAGLINECommand::Execute()
{
  if (!Valid(argStr))
  {
    control.Reply(ftp::ActionNotOkay, 
                  "Tagline most contain only printable "
                  "characters and none of the following: " +
                  charsNotAllowed);
    return cmd::Result::Okay;
  }

  util::Error ok = db::userprofile::SetTagline(client.User().UID(), argStr);
  if (!ok)
    control.Reply(ftp::ActionNotOkay, ok.Message());
  else
    control.Reply(ftp::CommandOkay, "New Tagline: " + argStr);
  
  return cmd::Result::Okay;
}

} /* site namespace */
} /* cmd namespace */
