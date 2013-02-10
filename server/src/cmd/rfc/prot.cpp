#include "cmd/rfc/prot.hpp"
#include "util/net/tlscontext.hpp"

namespace cmd { namespace rfc
{

void PROTCommand::Execute()
{
  if (args[1] == "P")
  {
    if (!util::net::TLSServerContext::Get() ||
        !util::net::TLSClientContext::Get())
    {
      control.Reply(ftp::ParameterNotImplemented,
                    "TLS is not enabled.");
      return;
    }
    
    data.SetProtection(true);
    control.Reply(ftp::CommandOkay, "Protection type set to 'private'.");
  }
  else if (args[1] == "C")
  {
    data.SetProtection(false);
    control.Reply(ftp::CommandOkay, "Protection type set to 'clear'.");
  }
  else if (args[1] == "S")
    control.Reply(ftp::ParameterNotImplemented,
                 "Protection type 'secure' not implemented.");
  else if (args[1] == "E")
    control.Reply(ftp::ParameterNotImplemented,
                 "Protection type 'confidential' not implemented.");
  else
    control.Reply(ftp::SyntaxError, "Unrecognised protection type.");
  return;
}

} /* rfc namespace */
} /* cmd namespace */
