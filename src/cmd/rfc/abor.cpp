#include "cmd/rfc/abor.hpp"

namespace cmd { namespace rfc
{

cmd::Result ABORCommand::Execute()
{
  control.Reply(ftp::DataClosedOkay, "ABOR command successful."); 
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
