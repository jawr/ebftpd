#include "cmd/rfc/quit.hpp"
#include "text/util.hpp"

namespace cmd { namespace rfc
{

void QUITCommand::Execute()
{
  std::string goodbye;
  if (client.State() == ftp::ClientState::LoggedIn)
  {
    if (text::GenericTemplate(client, "goodbye", goodbye))
      control.Reply(ftp::ClosingControl, goodbye);
  }
  
  if (goodbye.empty()) control.Reply(ftp::ClosingControl, "Bye bye");

  client.SetState(ftp::ClientState::Finished);
}

} /* rfc namespace */
} /* cmd namespace */
