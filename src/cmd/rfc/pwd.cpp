#include "cmd/rfc/pwd.hpp"
#include "fs/directory.hpp"
#include "fs/path.hpp"

namespace cmd { namespace rfc
{

void PWDCommand::Execute()
{
  control.Reply(ftp::PathCreated, "\"" + fs::WorkDirectory().ToString() +
               "\" is your working directory.");
}

} /* rfc namespace */
} /* cmd namespace */
