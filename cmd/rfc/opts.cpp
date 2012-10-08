#include "cmd/rfc/opts.hpp"

namespace cmd { namespace rfc
{

void OPTSCommand::Execute()
{
  control.Reply(ftp::NotImplemented, "OPTS Command not implemented."); 
}

} /* rfc namespace */
} /* cmd namespace */
