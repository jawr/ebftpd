#include "cmd/rfc/rest.hpp"

namespace cmd { namespace rfc
{

void RESTCommand::Execute()
{
  control.Reply(ftp::NotImplemented, "REST Command not implemented."); 
}

} /* rfc namespace */
} /* cmd namespace */
