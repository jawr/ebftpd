#include "cmd/rfc/cwd.hpp"
#include "fs/directory.hpp"

namespace cmd { namespace rfc
{

void CWDCommand::Execute()
{
  fs::VirtualPath path(fs::PathFromUser(argStr));
  
  util::Error e = fs::ChangeDirectory(client,  path);
  if (e)
  {
    control.Reply(ftp::FileActionOkay, "CWD command successful."); 
    return;
  }
  
  fs::VirtualPath match;
  if (e.Errno() == ENOENT && (e = fs::ChangeAlias(client, argStr, match)))
  {
    control.Reply(ftp::FileActionOkay, "CWD command successful (Alias: " + 
          match.ToString() + ").");
    return;
  }

  if (e.Errno() == ENOENT && (e = fs::ChangeMatch(client, path, match)))
  {
    control.Reply(ftp::FileActionOkay, "CWD command successful (Matched: " + 
                 match.ToString() + ").");
    return;
  }

  if (e.Errno() == ENOENT && (e = fs::ChangeCdpath(client, argStr, match)))
  {
    control.Reply(ftp::FileActionOkay, "CWD command successful (Matched: " + 
                 match.ToString() + ").");
    return;    
  }

  control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
  return;
}

} /* rfc namespace */
} /* cmd namespace */
