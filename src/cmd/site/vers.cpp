#include "cmd/site/vers.hpp"
#include "main.hpp"

namespace cmd { namespace site
{

cmd::Result VERSCommand::Execute()
{
  control.Reply(ftp::CommandOkay, "This server is running: " + programFullname);
  return cmd::Result::Okay;
}

} /* site namespace */
} /* cmd namespace */
