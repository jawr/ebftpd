#include "cmd/rfc/quit.hpp"

namespace cmd { namespace rfc
{

cmd::Result QUITCommand::Execute()
{
  control.Reply(ftp::ClosingControl, "Bye bye"); 
  client.SetState(ftp::ClientState::Finished);
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
