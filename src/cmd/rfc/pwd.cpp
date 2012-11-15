#include "cmd/rfc/pwd.hpp"

namespace cmd { namespace rfc
{

void PWDCommand::Execute()
{
  control.Reply(ftp::PathCreated, "\"" + client.WorkDir().ToString() +
               "\" is your working directory.");
  return;
}

} /* rfc namespace */
} /* cmd namespace */
