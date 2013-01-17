#include "cmd/rfc/pasv.hpp"
#include "util/net/ftp.hpp"

namespace cmd { namespace rfc
{

void PASVCommand::Execute()
{
  util::net::Endpoint ep;
  try
  {
    data.InitPassive(ep, ftp::PassiveType::PASV);
  }
  catch (const util::net::NetworkError& e)
  {
    control.Reply(ftp::CantOpenDataConnection,
                 "Unable to listen for data connection: " + e.Message());
    throw cmd::NoPostScriptError();
  }

  std::string portString;
  util::Error e = util::net::ftp::EndpointToPORT(ep, portString);
  if (!e)
  {
    control.Reply(ftp::SyntaxError, e.Message());
    throw cmd::NoPostScriptError();
  }
  
  control.Reply(ftp::PassiveMode, "Entering passive mode (" + portString + ")");
}

} /* rfc namespace */
} /* cmd namespace */
