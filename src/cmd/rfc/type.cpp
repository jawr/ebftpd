#include "cmd/rfc/type.hpp"

namespace cmd { namespace rfc
{

void TYPECommand::Execute()
{
  if (args[1] != "I" && args[1] != "A")
  {
    control.Reply(ftp::ParameterNotImplemented,
                 "TYPE " + args[1] + " not supported.");
    throw cmd::NoPostScriptError();
  }
  
  data.SetDataType(args[1] == "I" ? ftp::DataType::Binary : ftp::DataType::ASCII);
  
  control.Reply(ftp::CommandOkay, "TYPE command successful."); 
}

} /* rfc namespace */
} /* cmd namespace */
