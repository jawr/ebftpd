#include "cmd/rfc/lprt.hpp"
#include "util/net/ftp.hpp"

namespace cmd { namespace rfc
{

void LPRTCommand::Execute()
{
  if (args.size() != 2)
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }

  util::net::Endpoint ep;
  util::Error e = util::net::ftp::EndpointFromLPRT(args[1], ep);
  if (!e)
  {
    control.Reply(ftp::SyntaxError, "LPRT failed: " + e.Message());
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
}

} /* rfc namespace */
} /* cmd namespace */
