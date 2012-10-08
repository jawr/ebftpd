#include "cmd/rfc/acct.hpp"

namespace cmd { namespace rfc
{

void ACCTCommand::Execute()
{
  control.Reply(ftp::NotImplemented, "ACCT Command not implemented."); 
}

} /* rfc namespace */
} /* cmd namespace */
