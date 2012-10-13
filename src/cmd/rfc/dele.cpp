#include "cmd/rfc/dele.hpp"
#include "fs/file.hpp"

namespace cmd { namespace rfc
{

void DELECommand::Execute()
{
  if (argStr.empty())
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  util::Error e = fs::DeleteFile(client,  argStr);
  if (!e) control.Reply(ftp::ActionNotOkay, "DELE failed: " + e.Message());
  else control.Reply(ftp::FileActionOkay, "DELE command successful."); 
}

} /* rfc namespace */
} /* cmd namespace */
