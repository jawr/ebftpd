#include "cmd/rfc/noop.hpp"

namespace cmd { namespace rfc
{

cmd::Result NOOPCommand::Execute()
{
  control.Reply(ftp::CommandOkay, "NOOP command successful."); 
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
