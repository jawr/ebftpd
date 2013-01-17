#include "cmd/rfc/cdup.hpp"
#include "fs/directory.hpp"
#include "cmd/error.hpp"

namespace cmd { namespace rfc
{

void CDUPCommand::Execute()
{
  util::Error e = fs::ChangeDirectory(client,  fs::Resolve(fs::MakeVirtual("..")));
  if (!e)
  {
    control.Reply(ftp::ActionNotOkay, "..: " + e.Message());
    throw cmd::NoPostScriptError();
  }
  
  control.Reply(ftp::FileActionOkay, "CDUP command successful."); 
}

} /* rfc namespace */
} /* cmd namespace */
