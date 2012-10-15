#include "cmd/rfc/epsv.hpp"
#include "util/net/ftp.hpp"

namespace cmd { namespace rfc
{

cmd::Result EPSVCommand::Execute()
{
  util::net::Endpoint ep;
  try
  {
    data.InitPassive(ep, ftp::PassiveType::EPSV);
  }
  catch (const util::net::NetworkError& e)
  {
    control.Reply(ftp::CantOpenDataConnection,
                 "Unable to listen for data connection: " + e.Message());
    return cmd::Result::Okay;
  }

  std::string portString;
  util::net::ftp::EndpointToEPRT(ep, portString, 
                                 data.EPSVMode() == ftp::EPSVMode::Full);
  
  control.Reply(ftp::ExtendedPassiveMode, "Entering extended passive mode (" + 
               portString + ")");

  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
