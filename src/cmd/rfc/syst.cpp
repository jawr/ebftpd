#include "cmd/rfc/syst.hpp"

namespace cmd { namespace rfc
{

void SYSTCommand::Execute()
{
  control.Reply(ftp::SystemType, "UNIX Type: L8"); 
}

} /* rfc namespace */
} /* cmd namespace */
