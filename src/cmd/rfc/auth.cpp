#include "cmd/rfc/auth.hpp"
#include "util/net/tlscontext.hpp"
#include "ftp/data.hpp"

namespace cmd { namespace rfc
{

void AUTHCommand::Execute()
{
  if (!util::net::TLSServerContext::Get())
  {
    control.Reply(ftp::ParameterNotImplemented, "TLS is not enabled.");
    return;
  }
  
  if (args[1] == "SSL") data.SetProtection(true);
  else if (args[1] != "TLS")
  {
    control.Reply(ftp::ParameterNotImplemented,
                 "AUTH " + args[1] + " is unsupported.");
    return;
  }
  
  control.Reply(ftp::SecurityExchangeOkay, "AUTH " + args[1] + " successful."); 
  control.NegotiateTLS();
}

} /* rfc namespace */
} /* cmd namespace */
