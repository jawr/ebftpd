#include "cmd/rfc/pwd.hpp"

namespace cmd { namespace rfc
{

cmd::Result PWDCommand::Execute()
{
  control.Reply(ftp::PathCreated, "\"" + client.WorkDir().ToString() +
               "\" is your working directory.");
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
