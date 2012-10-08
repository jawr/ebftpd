#include "cmd/rfc/mkd.hpp"
#include "fs/directory.hpp"

namespace cmd { namespace rfc
{

void MKDCommand::Execute()
{
  if (argStr.empty())
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  util::Error e = fs::CreateDirectory(client,  argStr);
  if (!e) control.Reply(ftp::ActionNotOkay, "MKD failed: " + e.Message());
  else control.Reply(ftp::PathCreated, "MKD command successful."); 
}

} /* rfc namespace */
} /* cmd namespace */
