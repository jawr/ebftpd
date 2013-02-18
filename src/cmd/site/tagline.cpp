#include <cctype>
#include "cmd/site/tagline.hpp"
#include "acl/util.hpp"

namespace cmd { namespace site
{

void TAGLINECommand::Execute()
{
  if (!acl::Validate(acl::ValidationType::Tagline, argStr))
  {
    control.Reply(ftp::ActionNotOkay, "Tagline contains invalid characters");
    return;
  }

  client.User().SetTagline(argStr);
  control.Reply(ftp::CommandOkay, "New Tagline: " + argStr);
}

} /* site namespace */
} /* cmd namespace */
