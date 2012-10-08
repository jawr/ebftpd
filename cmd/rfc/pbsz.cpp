#include "cmd/rfc/pbsz.hpp"

namespace cmd { namespace rfc
{

void PBSZCommand::Execute()
{
  if (args.size() != 2)
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  if (args[1] != "0")
    control.Reply(ftp::ParameterNotImplemented, "Only protection buffer size 0 supported.");
  else
    control.Reply(ftp::CommandOkay, "Protection buffer size set to 0.");
}

} /* rfc namespace */
} /* cmd namespace */
