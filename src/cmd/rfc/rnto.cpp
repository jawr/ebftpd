#include "cmd/rfc/rnto.hpp"
#include "fs/file.hpp"
#include "acl/path.hpp"
#include "fs/directory.hpp"

namespace cmd { namespace rfc
{

void RNTOCommand::Execute()
{
  namespace PP = acl::path;
  
  fs::VirtualPath path(fs::PathFromUser(argStr));

  fs::Path messagePath;
  util::Error e(acl::path::Filter(client.User(), path.Basename(), messagePath));
  if (!e)
  {
    // should display above messagepath, we'll just reply for now
    control.Reply(ftp::ActionNotOkay, "Path name contains one or more invalid characters.");
    return;
  }

  try
  {
    if (fs::Status(client, client.RenameFrom()).IsDirectory())
      e = fs::RenameDirectory(client, client.RenameFrom(), path);
    else
      e = fs::RenameFile(client, client.RenameFrom(), path);    
      
    if (!e) control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
    else control.Reply(ftp::FileActionOkay, "RNTO command successful.");
  }
  catch (const util::SystemError& e)
  {
    control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
    return;
  }
}

} /* rfc namespace */
} /* cmd namespace */
