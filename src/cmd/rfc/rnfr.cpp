#include "cmd/rfc/rnfr.hpp"
#include "acl/path.hpp"
#include "cmd/error.hpp"
#include "util/status.hpp"

namespace cmd { namespace rfc
{

void RNFRCommand::Execute()
{
  fs::VirtualPath path(fs::PathFromUser(argStr));
  
  util::Error e(acl::path::FileAllowed<acl::path::View>(client.User(), path));
  if (!e)
  {
    control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
    throw cmd::NoPostScriptError();
  }

  try
  {
    util::path::Status status(path.ToString());
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
