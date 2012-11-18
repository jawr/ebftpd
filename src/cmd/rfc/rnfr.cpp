#include "cmd/rfc/rnfr.hpp"
#include "acl/path.hpp"

namespace cmd { namespace rfc
{

void RNFRCommand::Execute()
{
  namespace PP = acl::path;
  
  fs::VirtualPath path(fs::PathFromUser(argStr));
  
  util::Error e(PP::FileAllowed<PP::View>(client.User(), path));
  if (!e)
  {
    control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
    return;
  }

  try
  {
    fs::Status status(client, path);
  }
  catch (const util::SystemError& e)
  {
    control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
    return;
  }
  
  client.SetRenameFrom(path);
  control.Reply(ftp::PendingMoreInfo, "File exists, ready for destination name."); 
  return;
}

} /* rfc namespace */
} /* cmd namespace */
