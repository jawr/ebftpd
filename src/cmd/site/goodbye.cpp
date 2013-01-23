#include "cmd/site/goodbye.hpp"
#include "text/util.hpp"

namespace cmd { namespace site
{

void GOODBYECommand::Execute()
{
  std::string messages;
  util::Error e = text::GenericTemplate(client, "goodbye", messages);
  if (!e)
    control.Reply(ftp::ActionNotOkay, e.Message());
  else
    control.Reply(ftp::CommandOkay, messages);
}

} /* site namespace */
} /* cmd namespace */
