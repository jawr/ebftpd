#include "cmd/rfc/sscn.hpp"
#include "util/net/ftp.hpp"
#include "ftp/data.hpp"
#include "cmd/error.hpp"
#include "util/string.hpp"

namespace cmd { namespace rfc
{

void SSCNCommand::Execute()
{
  if (args.size() == 2)
  {
    util::ToLower(args[1]);
    if (args[1] == "on") data.SetSSCNMode(ftp::SSCNMode::Client);
    else
    if (args[1] == "off") data.SetSSCNMode(ftp::SSCNMode::Server);
    else
      throw cmd::SyntaxError();
  }

  std::stringstream os;
  os << "SSCN:";
  if (data.SSCNMode() == ftp::SSCNMode::Server) os << "SERVER METHOD";
  else os << "CLIENT METHOD";
  control.Reply(ftp::CommandOkay, os.str());  
}

} /* rfc namespace */
} /* cmd namespace */
