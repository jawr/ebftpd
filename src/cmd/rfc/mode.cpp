#include "cmd/rfc/mode.hpp"

namespace cmd { namespace rfc
{

void MODECommand::Execute()
{
  if (args.size() != 2)
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  if (args[1] == "S")
    control.Reply(ftp::CommandOkay, "Transfer mode set to 'stream'.");
  else if (args[1] == "B")
    control.Reply(ftp::ParameterNotImplemented,
                 "Transfer mode 'block' not implemented.");
  else if (args[1] == "C")
    control.Reply(ftp::ParameterNotImplemented,
                 "Transfer mode 'compressed' not implemented.");
  else
    control.Reply(ftp::SyntaxError, "Unrecognised transfer mode.");
}

} /* rfc namespace */
} /* cmd namespace */
