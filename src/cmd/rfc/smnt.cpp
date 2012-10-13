#include "cmd/rfc/smnt.hpp"

namespace cmd { namespace rfc
{

void SMNTCommand::Execute()
{
  control.Reply(ftp::NotImplemented, "SMNT Command not implemented."); 
}

} /* rfc namespace */
} /* cmd namespace */
