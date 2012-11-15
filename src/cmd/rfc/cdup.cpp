#include "cmd/rfc/cdup.hpp"
#include "fs/directory.hpp"
#include "cmd/error.hpp"

namespace cmd { namespace rfc
{

void CDUPCommand::Execute()
{
  if (!argStr.empty()) throw cmd::SyntaxError();
  
  fs::Path path = "..";
  
  util::Error e = fs::ChangeDirectory(client,  path);
  if (!e) control.Reply(ftp::ActionNotOkay, "..: " + e.Message());
  else control.Reply(ftp::FileActionOkay, "CDUP command successful."); 
  return;
}

} /* rfc namespace */
} /* cmd namespace */
