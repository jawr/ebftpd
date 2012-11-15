#include "cmd/rfc/cwd.hpp"
#include "fs/directory.hpp"

namespace cmd { namespace rfc
{

void CWDCommand::Execute()
{
  fs::Path path(argStr);
  
  util::Error e = fs::ChangeDirectory(client,  path);
  if (e)
  {
    control.Reply(ftp::FileActionOkay, "CWD command successful."); 
    return;
  }

  if (e.Errno() == ENOENT)
  {
    e = fs::ChangeAlias(client, path);
    if (e)
    {
      control.Reply(ftp::FileActionOkay, "CWD command successful (Alias: " + 
            path.ToString() + ").");
      return;
    }
    
    if (e.Errno() == ENOENT)
    {
      e = fs::ChangeMatch(client, path);
      if (e)
      {
        control.Reply(ftp::FileActionOkay, "CWD command successful (Matched: " + 
                     path.ToString() + ").");
        return;
      }
    }
  }
    
  control.Reply(ftp::ActionNotOkay, argStr + ":" + e.Message());
  return;
}

} /* rfc namespace */
} /* cmd namespace */
