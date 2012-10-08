#include "cmd/rfc/appe.hpp"

namespace cmd { namespace rfc
{

void APPECommand::Execute()
{
  control.Reply(ftp::NotImplemented, "APPE Command not implemented."); 
}

} /* rfc namespace */
} /* cmd namespace */
