#include "cmd/rfc/rnfr.hpp"
#include "acl/path.hpp"
#include "cmd/error.hpp"

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
    throw cmd::NoPostScriptError();
  }

  try
  {
    fs::Status status(client.User(), path);
  }
  catch (const util::SystemError& e)
  {
    control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
    throw cmd::NoPostScriptError();
  }
  
  client.SetRenameFrom(path);
  control.Reply(ftp::PendingMoreInfo, "File exists, ready for destination name."); 
}

} /* rfc namespace */
} /* cmd namespace */
