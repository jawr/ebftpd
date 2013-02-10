#include "cmd/rfc/noop.hpp"

namespace cmd { namespace rfc
{

void NOOPCommand::Execute()
{
  control.Reply(ftp::CommandOkay, "NOOP command successful."); 
  return;
}

} /* rfc namespace */
} /* cmd namespace */
