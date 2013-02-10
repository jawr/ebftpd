#include "cmd/rfc/abor.hpp"

namespace cmd { namespace rfc
{

void ABORCommand::Execute()
{
  control.Reply(ftp::DataClosedOkay, "ABOR command successful."); 
  return;
}

} /* rfc namespace */
} /* cmd namespace */
