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

void TAGLINECommand::Execute()
{
  if (!Valid(argStr))
  {
    control.Reply(ftp::ActionNotOkay, 
                  "Tagline most contain only printable "
                  "characters and none of the following: " +
                  charsNotAllowed);
    return;
  }

  util::Error ok = acl::UserCache::SetTagline(client.User().Name(), argStr);
  if (!ok)
    control.Reply(ftp::ActionNotOkay, ok.Message());
  else
    control.Reply(ftp::CommandOkay, "New Tagline: " + argStr);
  
  return;
}

} /* site namespace */
} /* cmd namespace */
