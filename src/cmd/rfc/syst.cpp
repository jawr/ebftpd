#include "cmd/rfc/syst.hpp"

namespace cmd { namespace rfc
{

cmd::Result SYSTCommand::Execute()
{
  control.Reply(ftp::SystemType, "UNIX Type: L8"); 
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
