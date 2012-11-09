#include <sstream>
#include "cmd/rfc/feat.hpp"
#include "util/scopeguard.hpp"

namespace cmd { namespace rfc
{

cmd::Result FEATCommand::Execute()
{
  using util::scope_guard;
  using util::make_guard;

  bool singleLineReplies = control.SingleLineReplies();
  control.SetSingleLineReplies(false);
  
  scope_guard singleLineGuard = make_guard([&]{ control.SetSingleLineReplies(singleLineReplies); });  

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
  control.PartReply(ftp::NoCode, " CPSV");
  control.Reply(ftp::SystemStatus, "End.");
  return cmd::Result::Okay;
  
  (void) singleLineReplies;
  (void) singleLineGuard;
}

} /* rfc namespace */
} /* cmd namespace */
