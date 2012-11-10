#include "cmd/rfc/rnto.hpp"
#include "fs/file.hpp"
#include "acl/path.hpp"

namespace cmd { namespace rfc
{

cmd::Result RNTOCommand::Execute()
{
  std::string messagePath;
  util::Error e(acl::path::Filter(client.User(), fs::Path(argStr).Basename(), messagePath));
  if (!e)
  {
    // should display above messagepath, we'll just reply for now
    control.Reply(ftp::ActionNotOkay, "Path name contains one or more invalid characters.");
    return cmd::Result::Okay;
  }

  e = fs::RenameFile(client, client.RenameFrom(), argStr);
  if (!e) control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
  else control.Reply(ftp::FileActionOkay, "RNTO command successful.");
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
