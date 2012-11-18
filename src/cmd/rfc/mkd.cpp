#include "cmd/rfc/mkd.hpp"
#include "fs/directory.hpp"
#include "acl/path.hpp"

namespace cmd { namespace rfc
{

void MKDCommand::Execute()
{
  fs::VirtualPath path(fs::PathFromUser(argStr));

  fs::Path messagePath;
  util::Error e(acl::path::Filter(client.User(), path.Basename(), messagePath));
  if (!e)
  {
    // should display above messagepath, we'll just reply for now
    control.Reply(ftp::ActionNotOkay, "Directory name contains one or more invalid characters.");
    return;
  }

  e = fs::CreateDirectory(client,  path);
  if (!e) control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
  else control.Reply(ftp::PathCreated, "MKD command successful."); 
  return;
}

} /* rfc namespace */
} /* cmd namespace */
