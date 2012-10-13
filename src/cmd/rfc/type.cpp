#include "cmd/rfc/type.hpp"

namespace cmd { namespace rfc
{

void TYPECommand::Execute()
{
  if (args.size() != 2)
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  if (args[1] != "I" && args[1] != "A")
  {
    control.Reply(ftp::ParameterNotImplemented,
                 "TYPE " + args[1] + " not supported.");
    return;
  }
  
  control.Reply(ftp::CommandOkay, "TYPE command successful."); 
}

} /* rfc namespace */
} /* cmd namespace */
