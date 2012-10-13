#include "cmd/rfc/cdup.hpp"
#include "fs/directory.hpp"

namespace cmd { namespace rfc
{

void CDUPCommand::Execute()
{
  if (!argStr.empty())
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  fs::Path path = "..";
  
  util::Error e = fs::ChangeDirectory(client,  path);
  if (!e) control.Reply(ftp::ActionNotOkay, "CDUP failed: " + e.Message());
  else control.Reply(ftp::FileActionOkay, "CDUP command successful."); 
}

} /* rfc namespace */
} /* cmd namespace */
