#include "cmd/rfc/cpsv.hpp"
#include "util/net/ftp.hpp"
#include "ftp/data.hpp"

namespace cmd { namespace rfc
{

void CPSVCommand::Execute()
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
    return;
  }

  std::string portString;
  util::Error e = util::net::ftp::EndpointToPORT(ep, portString);
  if (!e)
  {
    control.Reply(ftp::SyntaxError, e.Message());
    return;
  }
  
  control.Reply(ftp::PassiveMode, "Entering passive mode (" + portString + ")");
}

} /* rfc namespace */
} /* cmd namespace */
