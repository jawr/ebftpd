#include <cctype>
#include "cmd/site/tagline.hpp"
#include "db/user/userprofile.hpp"
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

  db::userprofile::SetTagline(client.User().UID(), argStr);
  control.Reply(ftp::CommandOkay, "New Tagline: " + argStr);
}

} /* site namespace */
} /* cmd namespace */
