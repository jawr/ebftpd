#include "cmd/rfc/mlst.hpp"

namespace cmd { namespace rfc
{

void MLSTCommand::Execute()
{
  control.Reply(ftp::NotImplemented, "MLST Command not implemented."); 
}

} /* rfc namespace */
} /* cmd namespace */
