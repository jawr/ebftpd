#include "cmd/rfc/ccc.hpp"

namespace cmd { namespace rfc
{

void CCCCommand::Execute()
{
  control.Reply(ftp::NotImplemented, "CCC Command not implemented."); 
}

} /* rfc namespace */
} /* cmd namespace */
