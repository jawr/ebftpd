#include "cmd/rfc/allo.hpp"

namespace cmd { namespace rfc
{

void ALLOCommand::Execute()
{
  control.Reply(ftp::NotImplemented, "ALLO Command not implemented."); 
}

} /* rfc namespace */
} /* cmd namespace */
