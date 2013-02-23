#include <cctype>
#include "cmd/site/tagline.hpp"
#include "acl/util.hpp"
#include "acl/group.hpp"
#include "logs/logs.hpp"
#include "acl/user.hpp"

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

  logs::Event("TAGLINE", client.User().Name(), client.User().PrimaryGroup(), client.User().Tagline());
}

} /* site namespace */
} /* cmd namespace */
