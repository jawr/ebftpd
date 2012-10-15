#include "cmd/rfc/lpsv.hpp"
#include "util/net/ftp.hpp"

namespace cmd { namespace rfc
{

cmd::Result LPSVCommand::Execute()
{
  util::net::Endpoint ep;
  try
  {
    data.InitPassive(ep, ftp::PassiveType::LPSV);
  }
  catch (const util::net::NetworkError& e)
  {
    control.Reply(ftp::CantOpenDataConnection,
                 "Unable to listen for data connection: " + e.Message());
    return cmd::Result::Okay;
  }
  
  std::string portString;
  util::net::ftp::EndpointToLPRT(ep, portString);
  
  control.Reply(ftp::LongPassiveMode, "Entering passive mode (" + portString + ")");
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
