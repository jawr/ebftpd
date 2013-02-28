#include "cmd/rfc/epsv.hpp"
#include "util/net/ftp.hpp"
#include "cfg/get.hpp"
#include "cmd/error.hpp"
#include "util/string.hpp"
#include "ftp/data.hpp"

namespace cmd { namespace rfc
{

void EPSVCommand::ChangeMode()
{
  auto epsvFxp = cfg::Get().EPSVFxp();
  util::ToUpper(args[1]);
  if (args[1] == "EXTENDED") 
  {
    if (epsvFxp == ::cfg::EPSVFxp::Deny)
    {
      control.Reply(ftp::ParameterNotImplemented, "EPSV EXTENDED not supported.");
      return;
    }
    client.Data().SetEPSVMode(ftp::EPSVMode::Extended);
  }
  else if (args[1] == "NORMAL") 
  {
    if (epsvFxp == ::cfg::EPSVFxp::Force)
    {
      control.Reply(ftp::ParameterNotImplemented, "EPSV NORMAL not supported.");
      return;
    }
    client.Data().SetEPSVMode(ftp::EPSVMode::Normal);
  }
  else if (args[1] != "MODE")
    throw cmd::SyntaxError();
  
  std::ostringstream os;
  os << util::EnumToString(epsvFxp) << " (" 
     << util::EnumToString(client.Data().EPSVMode()) << ").";
  control.Reply(ftp::CommandOkay, os.str());
}

void EPSVCommand::Execute()
{
  if (args.size() == 2)
  {
    ChangeMode();
    return;
  }

  util::net::Endpoint ep;
  try
  {
    data.InitPassive(ep, ftp::PassiveType::EPSV);
  }
  catch (const util::net::NetworkError& e)
  {
    control.Reply(ftp::CantOpenDataConnection,
                 "Unable to listen for data connection: " + e.Message());
    return;
  }

  std::string portString;
  util::net::ftp::EndpointToEPRT(ep, portString, 
                                 data.EPSVMode() == ftp::EPSVMode::Extended);
  
  control.Reply(ftp::ExtendedPassiveMode, "Entering extended passive mode (" + 
               portString + ")");

  return;
}

} /* rfc namespace */
} /* cmd namespace */
