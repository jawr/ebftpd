#include "cmd/rfc/eprt.hpp"
#include "util/net/ftp.hpp"

namespace cmd { namespace rfc
{

cmd::Result EPRTCommand::Execute()
{
  util::net::Endpoint ep;
  util::Error e = util::net::ftp::EndpointFromEPRT(args[1], ep);
  if (!e)
  {
    control.Reply(ftp::SyntaxError, "EPRT failed: " + e.Message());
    return cmd::Result::Okay;
  }
  
  try
  {
    data.InitActive(ep);
  }
  catch (const util::net::NetworkError& e)
  {
    control.Reply(ftp::CantOpenDataConnection,
                 "Unable to open data connection: " + e.Message());
    return cmd::Result::Okay;
  }
  
  control.Reply(ftp::CommandOkay, "EPRT command successful.");
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
