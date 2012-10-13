#include "cmd/rfc/conf.hpp"

namespace cmd { namespace rfc
{

void CONFCommand::Execute()
{
  control.Reply(ftp::NotImplemented, "CONF Command not implemented."); 
}

} /* rfc namespace */
} /* cmd namespace */
