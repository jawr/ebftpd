#include "cmd/rfc/rmd.hpp"
#include "fs/directory.hpp"

namespace cmd { namespace rfc
{

cmd::Result RMDCommand::Execute()
{
  util::Error e = fs::RemoveDirectory(client,  argStr);
  if (!e) control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
  else control.Reply(ftp::FileActionOkay, "RMD command successful."); 
  return Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
