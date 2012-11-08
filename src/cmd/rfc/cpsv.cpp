#include "cmd/rfc/cpsv.hpp"
#include "util/net/ftp.hpp"

namespace cmd { namespace rfc
{

cmd::Result CPSVCommand::Execute()
{
  util::net::Endpoint ep;
  try
  {
    data.InitPassive(ep, ftp::PassiveType::CPSV);
  }
  catch (const util::net::NetworkError& e)
  {
    control.Reply(ftp::CantOpenDataConnection,
                 "Unable to listen for data connection: " + e.Message());
    return cmd::Result::Okay;
  }

  std::string portString;
  util::Error e = util::net::ftp::EndpointToPORT(ep, portString);
  if (!e)
  {
    control.Reply(ftp::SyntaxError, e.Message());
    return cmd::Result::Okay;
  }
  
  control.Reply(ftp::PassiveMode, "Entering passive mode (" + portString + ")");
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
