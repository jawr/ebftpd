#include <sstream>
#include "cmd/rfc/feat.hpp"

namespace cmd { namespace rfc
{

cmd::Result FEATCommand::Execute()
{
  std::ostringstream os;
  os << "Extended feature support:\n"
     << " AUTH TLS\n"
     << " EPRT\n"
     << " EPSV\n"
     << " LPRT\n"
     << " LPSV\n"
     << " PBSZ\n"
     << " PROT\n"
     << " MDTM\n"
     << " SIZE\n"
     << "End.";
  control.MultiReply(ftp::SystemStatus, os.str());
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
