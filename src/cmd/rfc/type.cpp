#include "cmd/rfc/type.hpp"

namespace cmd { namespace rfc
{

cmd::Result TYPECommand::Execute()
{
  if (args[1] != "I" && args[1] != "A")
  {
    control.Reply(ftp::ParameterNotImplemented,
                 "TYPE " + args[1] + " not supported.");
    return cmd::Result::Okay;
  }
  
  control.Reply(ftp::CommandOkay, "TYPE command successful."); 
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
