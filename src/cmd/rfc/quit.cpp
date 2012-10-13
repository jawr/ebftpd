#include "cmd/rfc/quit.hpp"

namespace cmd { namespace rfc
{

void QUITCommand::Execute()
{
  control.Reply(ftp::ClosingControl, "Bye bye"); 
  client.SetFinished();
}

} /* rfc namespace */
} /* cmd namespace */
