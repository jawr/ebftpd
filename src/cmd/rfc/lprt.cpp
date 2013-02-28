#include "cmd/rfc/lprt.hpp"
#include "util/net/ftp.hpp"
#include "ftp/data.hpp"

namespace cmd { namespace rfc
{

void LPRTCommand::Execute()
{
  util::net::Endpoint ep;
  util::Error e = util::net::ftp::EndpointFromLPRT(args[1], ep);
  if (!e)
  {
    control.Reply(ftp::SyntaxError, args[1] + ": " + e.Message());
    return;
  }
  
  try
  {
    data.InitActive(ep);
  }
  catch (const util::net::NetworkError& e)
  {
    control.Reply(ftp::CantOpenDataConnection,
                 "Unable to open data connection: " + e.Message());
    return;
  }
  
  control.Reply(ftp::CommandOkay, "LPRT command successful.");
  return;
}

} /* rfc namespace */
} /* cmd namespace */
