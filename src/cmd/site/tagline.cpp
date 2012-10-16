#include <cctype>
#include "cmd/site/tagline.hpp"
#include "acl/usercache.hpp"

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
  
  control.Reply(ftp::NotImplemented, "Not implemented");
  return cmd::Result::Okay;
}

} /* site namespace */
} /* cmd namespace */
