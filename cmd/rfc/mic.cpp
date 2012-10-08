#include "cmd/rfc/mic.hpp"

namespace cmd { namespace rfc
{

void MICCommand::Execute()
{
  control.Reply(ftp::NotImplemented, "MIC Command not implemented."); 
}

} /* rfc namespace */
} /* cmd namespace */
