#include "cmd/rfc/quit.hpp"
#include "text/util.hpp"

namespace cmd { namespace rfc
{

void QUITCommand::Execute()
{
  std::string goodbye;
  if (text::GenericTemplate("goodbye", goodbye))
    control.Reply(ftp::ClosingControl, goodbye);
  else control.Reply(ftp::ClosingControl, "Bye bye");

  client.SetState(ftp::ClientState::Finished);
}

} /* rfc namespace */
} /* cmd namespace */
