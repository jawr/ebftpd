#include "cmd/rfc/lpsv.hpp"
#include "util/net/ftp.hpp"

namespace cmd { namespace rfc
{

void LPSVCommand::Execute()
{
  if (!argStr.empty())
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  util::net::Endpoint ep;
  try
  {
    data.InitPassive(ep, ftp::PassiveType::LPSV);
  }
  catch (const util::net::NetworkError& e)
  {
    control.Reply(ftp::CantOpenDataConnection,
                 "Unable to listen for data connection: " + e.Message());
    return;
  }
  
  std::string portString;
  util::net::ftp::EndpointToLPRT(ep, portString);
  
  control.Reply(ftp::LongPassiveMode, "Entering passive mode (" + portString + ")");
}

} /* rfc namespace */
} /* cmd namespace */
