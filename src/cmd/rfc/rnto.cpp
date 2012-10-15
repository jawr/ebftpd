#include "cmd/rfc/rnto.hpp"
#include "fs/file.hpp"

namespace cmd { namespace rfc
{

cmd::Result RNTOCommand::Execute()
{
  util::Error e = fs::RenameFile(client, client.RenameFrom(), argStr);
  if (!e) control.Reply(ftp::ActionNotOkay, "RNTO failed: " + e.Message());
  else control.Reply(ftp::FileActionOkay, "RNTO command successful.");
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
