#include "cmd/site/welcome.hpp"
#include "text/util.hpp"
#include "acl/misc.hpp"
#include "logs/logs.hpp"

namespace cmd { namespace site
{

void WELCOMECommand::Execute()
{
  fs::Path welcomePath(acl::message::Choose<acl::message::Welcome>(client.User()));
  if (!welcomePath.IsEmpty())
  {
    std::string welcome;
    auto e = text::GenericTemplate(client, welcomePath, welcome);
    if (!e) logs::Error("Failed to display welcome message: %1%", e.Message());
    else
    {
      control.Reply(ftp::CommandOkay, welcome);
      return;
    }
  }
  
  control.Reply(ftp::CommandOkay, "No welcome message");
}

} /* site namespace */
} /* cmd namespace */
