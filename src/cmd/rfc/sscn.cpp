#include "cmd/rfc/sscn.hpp"
#include "util/net/ftp.hpp"
#include "ftp/data.hpp"

namespace cmd { namespace rfc
{

cmd::Result SSCNCommand::Execute()
{
  if (args.size() == 2)
  {
    boost::to_lower(args[1]);
    if (args[1] == "on") data.SetSSCNMode(ftp::SSCNMode::Client);
    else
    if (args[1] == "off") data.SetSSCNMode(ftp::SSCNMode::Server);
    else
      return cmd::Result::SyntaxError;
  }

  std::stringstream os;
  os << "SSCN:";
  if (data.SSCNMode() == ftp::SSCNMode::Server) os << "SERVER METHOD";
  else os << "CLIENT METHOD";
  control.Reply(ftp::CommandOkay, os.str());  
  
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
