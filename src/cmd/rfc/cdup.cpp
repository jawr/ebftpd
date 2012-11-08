#include "cmd/rfc/cdup.hpp"
#include "fs/directory.hpp"

namespace cmd { namespace rfc
{

cmd::Result CDUPCommand::Execute()
{
  if (!argStr.empty()) return cmd::Result::SyntaxError;
  
  fs::Path path = "..";
  
  util::Error e = fs::ChangeDirectory(client,  path);
  if (!e) control.Reply(ftp::ActionNotOkay, "..: " + e.Message());
  else control.Reply(ftp::FileActionOkay, "CDUP command successful."); 
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
