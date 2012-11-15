#include "cmd/rfc/auth.hpp"

namespace cmd { namespace rfc
{

void AUTHCommand::Execute()
{
  if (args[1] == "SSL") data.SetProtection(true);
  else if (args[1] != "TLS")
  {
    control.Reply(ftp::ParameterNotImplemented,
                 "AUTH " + args[1] + " is unsupported.");
    return;
  }
  
  control.Reply(ftp::SecurityExchangeOkay, "AUTH " + args[1] + " successful."); 
  control.NegotiateTLS();

  return;
}

} /* rfc namespace */
} /* cmd namespace */
