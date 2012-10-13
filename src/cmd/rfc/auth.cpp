#include "cmd/rfc/auth.hpp"

namespace cmd { namespace rfc
{

void AUTHCommand::Execute()
{
  if (argStr.empty())
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  if (argStr != "TLS" && argStr != "SSL")
  {
    control.Reply(ftp::ParameterNotImplemented,
                 "AUTH " + argStr + " is unsupported.");
    return;
  }
  
  control.Reply(ftp::SecurityExchangeOkay, "AUTH TLS successful."); 
  control.NegotiateTLS();  
}

} /* rfc namespace */
} /* cmd namespace */
