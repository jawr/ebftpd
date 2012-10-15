#include "cmd/rfc/mkd.hpp"
#include "fs/directory.hpp"

namespace cmd { namespace rfc
{

cmd::Result MKDCommand::Execute()
{
  util::Error e = fs::CreateDirectory(client,  argStr);
  if (!e) control.Reply(ftp::ActionNotOkay, "MKD failed: " + e.Message());
  else control.Reply(ftp::PathCreated, "MKD command successful."); 
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
