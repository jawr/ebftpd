#include "cmd/rfc/eprt.hpp"
#include "util/net/ftp.hpp"

namespace cmd { namespace rfc
{

void EPRTCommand::Execute()
{
  if (args.size() != 2)
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }

  util::net::Endpoint ep;
  util::Error e = util::net::ftp::EndpointFromEPRT(args[1], ep);
  if (!e)
  {
    control.Reply(ftp::SyntaxError, "EPRT failed: " + e.Message());
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
  
  control.Reply(ftp::CommandOkay, "EPRT command successful.");
}

} /* rfc namespace */
} /* cmd namespace */
