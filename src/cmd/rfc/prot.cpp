#include "cmd/rfc/prot.hpp"

namespace cmd { namespace rfc
{

void PROTCommand::Execute()
{
  if (args.size() != 2)
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  if (args[1] == "P")
  {
    data.SetProtection(true);
    control.Reply(ftp::CommandOkay, "Protection type set to 'private'.");
  }
  else if (args[1] == "C")
  {
    data.SetProtection(false);
    control.Reply(ftp::CommandOkay, "Protection type set to 'clear'.");
  }
  else if (args[1] == "S")
    control.Reply(ftp::ParameterNotImplemented,
                 "Protection type 'secure' not implemented.");
  else if (args[1] == "E")
    control.Reply(ftp::ParameterNotImplemented,
                 "Protection type 'confidential' not implemented.");
  else
    control.Reply(ftp::SyntaxError, "Unrecognised protection type.");
}

} /* rfc namespace */
} /* cmd namespace */
