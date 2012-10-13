#include "cmd/rfc/cwd.hpp"
#include "fs/directory.hpp"

namespace cmd { namespace rfc
{

void CWDCommand::Execute()
{
  if (argStr.empty())
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  fs::Path path = argStr;
  
  util::Error e = fs::ChangeDirectory(client,  path);
  if (!e) control.Reply(ftp::ActionNotOkay, "CWD failed: " + e.Message());
  else if (path.ToString() != argStr)
    control.Reply(ftp::FileActionOkay, "CWD command successful (Matched: " + 
                 path.ToString() + ").");
  else
    control.Reply(ftp::FileActionOkay, "CWD command successful."); 
}

} /* rfc namespace */
} /* cmd namespace */
