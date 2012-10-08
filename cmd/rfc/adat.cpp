#include "cmd/rfc/adat.hpp"

namespace cmd { namespace rfc
{

void ADATCommand::Execute()
{
  control.Reply(ftp::NotImplemented, "ADAT Command not implemented."); 
}

} /* rfc namespace */
} /* cmd namespace */
