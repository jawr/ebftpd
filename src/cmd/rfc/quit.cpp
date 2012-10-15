#include "cmd/rfc/quit.hpp"

namespace cmd { namespace rfc
{

cmd::Result QUITCommand::Execute()
{
  control.Reply(ftp::ClosingControl, "Bye bye"); 
  client.SetFinished();
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
