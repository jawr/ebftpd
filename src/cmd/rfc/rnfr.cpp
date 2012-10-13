#include "cmd/rfc/rnfr.hpp"
#include "acl/check.hpp"

namespace cmd { namespace rfc
{

void RNFRCommand::Execute()
{
  namespace PP = acl::PathPermission;
  
  if (argStr.empty())
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }

  fs::Path absolute = (client.WorkDir() / argStr).Expand();
  
  util::Error e(PP::FileAllowed<PP::Rename>(client.User(), absolute));
  if (!e)
  {
    control.Reply(ftp::ActionNotOkay, "RNFR failed: " + e.Message());
    return;
  }

  try
  {
    fs::Status status(absolute);
  }
  catch (const util::SystemError& e)
  {
    control.Reply(ftp::ActionNotOkay, "RNFR failed: " + e.Message());
    return;
  }
  
  control.PartReply(ftp::PendingMoreInfo, std::string(absolute));
  client.SetRenameFrom(absolute);
  control.Reply("File exists, ready for destination name."); 
}

} /* rfc namespace */
} /* cmd namespace */
