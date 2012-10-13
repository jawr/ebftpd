#include "cmd/rfc/lang.hpp"

namespace cmd { namespace rfc
{

void LANGCommand::Execute()
{
  control.Reply(ftp::NotImplemented, "LANG Command not implemented."); 
}

} /* rfc namespace */
} /* cmd namespace */
