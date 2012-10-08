#include "cmd/rfc/stru.hpp"

namespace cmd { namespace rfc
{

void STRUCommand::Execute()
{
  control.Reply(ftp::NotImplemented, "STRU Command not implemented."); 
}

} /* rfc namespace */
} /* cmd namespace */
