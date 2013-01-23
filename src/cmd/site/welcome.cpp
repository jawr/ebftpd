#include "cmd/site/welcome.hpp"
#include "text/util.hpp"

namespace cmd { namespace site
{

void WELCOMECommand::Execute()
{
  std::string messages;
  util::Error e = text::GenericTemplate(client, "welcome", messages);
  if (!e)
    control.Reply(ftp::ActionNotOkay, e.Message());
  else
    control.Reply(ftp::CommandOkay, messages);
}

} /* site namespace */
} /* cmd namespace */
