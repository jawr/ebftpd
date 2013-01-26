#include <cctype>
#include "cmd/site/tagline.hpp"
#include "acl/usercache.hpp"
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

  util::Error ok = acl::UserCache::SetTagline(client.User().Name(), argStr);
  if (!ok)
    control.Reply(ftp::ActionNotOkay, ok.Message());
  else
    control.Reply(ftp::CommandOkay, "New Tagline: " + argStr);
  
  return;
}

} /* site namespace */
} /* cmd namespace */
