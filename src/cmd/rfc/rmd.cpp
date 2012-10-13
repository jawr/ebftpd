#include "cmd/rfc/rmd.hpp"
#include "fs/directory.hpp"

namespace cmd { namespace rfc
{

void RMDCommand::Execute()
{
  if (argStr.empty())
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  util::Error e = fs::RemoveDirectory(client,  argStr);
  if (!e) control.Reply(ftp::ActionNotOkay, "RMD failed: " + e.Message());
  else control.Reply(ftp::FileActionOkay, "RMD command successful."); 
}

} /* rfc namespace */
} /* cmd namespace */
