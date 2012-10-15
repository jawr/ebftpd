#include "cmd/rfc/dele.hpp"
#include "fs/file.hpp"

namespace cmd { namespace rfc
{

cmd::Result DELECommand::Execute()
{
  util::Error e = fs::DeleteFile(client,  argStr);
  if (!e) control.Reply(ftp::ActionNotOkay, "DELE failed: " + e.Message());
  else control.Reply(ftp::FileActionOkay, "DELE command successful."); 
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
