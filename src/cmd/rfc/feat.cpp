#include <sstream>
#include "cmd/rfc/feat.hpp"

namespace cmd { namespace rfc
{

cmd::Result FEATCommand::Execute()
{
  control.PartReply(ftp::SystemStatus, "Extended feature support:");
  control.PartReply(ftp::NoCode, " AUTH TLS");
  control.PartReply(ftp::NoCode, " EPRT");
  control.PartReply(ftp::NoCode, " EPSV");
  control.PartReply(ftp::NoCode, " LPRT");
  control.PartReply(ftp::NoCode, " LPSV");
  control.PartReply(ftp::NoCode, " PBSZ");
  control.PartReply(ftp::NoCode, " PROT");
  control.PartReply(ftp::NoCode, " MDTM");
  control.PartReply(ftp::NoCode, " SIZE");
  control.PartReply(ftp::NoCode, " SSCN");
  control.Reply(ftp::SystemStatus, "End.");
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
