#include "cmd/rfc/rnfr.hpp"
#include "acl/path.hpp"

namespace cmd { namespace rfc
{

cmd::Result RNFRCommand::Execute()
{
  namespace PP = acl::path;
  
  fs::Path absolute = (client.WorkDir() / argStr).Expand();
  
  util::Error e(PP::FileAllowed<PP::Rename>(client.User(), absolute));
  if (!e)
  {
    control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
    return cmd::Result::Okay;
  }

  try
  {
    fs::Status status(absolute);
  }
  catch (const util::SystemError& e)
  {
    control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
    return cmd::Result::Okay;
  }
  
  client.SetRenameFrom(absolute);
  control.Reply(ftp::PendingMoreInfo, "File exists, ready for destination name."); 
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
