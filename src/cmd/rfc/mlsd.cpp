#include "cmd/rfc/mlsd.hpp"

namespace cmd { namespace rfc
{

void MLSDCommand::Execute()
{
  control.Reply(ftp::NotImplemented, "MLSD Command not implemented."); 
}

} /* rfc namespace */
} /* cmd namespace */
