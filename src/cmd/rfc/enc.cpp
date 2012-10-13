#include "cmd/rfc/enc.hpp"

namespace cmd { namespace rfc
{

void ENCCommand::Execute()
{
  control.Reply(ftp::NotImplemented, "ENC Command not implemented."); 
}

} /* rfc namespace */
} /* cmd namespace */
