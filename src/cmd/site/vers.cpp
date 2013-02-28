#include "cmd/site/vers.hpp"
#include "main.hpp"

namespace cmd { namespace site
{

void VERSCommand::Execute()
{
  control.Reply(ftp::CommandOkay, "This server is running: " + programFullname);
  return;
}

} /* site namespace */
} /* cmd namespace */
