#include "cmd/rfc/rein.hpp"

namespace cmd { namespace rfc
{

void REINCommand::Execute()
{
  control.Reply(ftp::NotImplemented, "REIN Command not implemented."); 
}

} /* rfc namespace */
} /* cmd namespace */
