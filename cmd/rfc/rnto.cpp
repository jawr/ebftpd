#include "cmd/rfc/rnto.hpp"
#include "fs/file.hpp"

namespace cmd { namespace rfc
{

void RNTOCommand::Execute()
{
  if (argStr.empty())
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  util::Error e = fs::RenameFile(client, client.RenameFrom(), argStr);
  if (!e) control.Reply(ftp::ActionNotOkay, "RNTO failed: " + e.Message());
  else control.Reply(ftp::FileActionOkay, "RNTO command successful.");
}

} /* rfc namespace */
} /* cmd namespace */
