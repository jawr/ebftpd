#include "cmd/rfc/rmd.hpp"
#include "fs/directory.hpp"

namespace cmd { namespace rfc
{

void RMDCommand::Execute()
{
  fs::VirtualPath path(fs::Resolve(fs::VirtualPath(argStr)));
  util::Error e = fs::RemoveDirectory(client,  path);
  if (!e) control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
  else control.Reply(ftp::FileActionOkay, "RMD command successful."); 
}

} /* rfc namespace */
} /* cmd namespace */
