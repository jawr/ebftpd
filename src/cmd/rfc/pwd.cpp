#include "cmd/rfc/pwd.hpp"
#include "fs/directory.hpp"

namespace cmd { namespace rfc
{

void PWDCommand::Execute()
{
  control.Reply(ftp::PathCreated, "\"" + fs::WorkDirectory().ToString() +
               "\" is your working directory.");
  return;
}

} /* rfc namespace */
} /* cmd namespace */
