#include "cmd/rfc/mdtm.hpp"
#include "acl/path.hpp"
#include "cfg/get.hpp"
#include "fs/status.hpp"

namespace cmd { namespace rfc
{

void MDTMCommand::Execute()
{
  namespace PP = acl::path;

  fs::VirtualPath path(fs::PathFromUser(argStr));
  
  util::Error e(PP::FileAllowed<PP::View>(client.User(), path));
  if (!e)
  {
    control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
    return;
  }
  
  fs::Status status;
  try
  {
    status.Reset(fs::MakeReal(path));
  }
  catch (const util::SystemError& e)
  {
    control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
    return;
  }

  if (!status.IsRegularFile())
  {
    control.Reply(ftp::ActionNotOkay, argStr + ": Not a plain file.");
    return;
  }
  
  char timestamp[15];
  strftime(timestamp, sizeof(timestamp), "%Y%m%d%H%M%S",
           localtime(&status.Native().st_mtime));
  control.Reply(ftp::FileStatus, timestamp);
}

} /* rfc namespace */
} /* cmd namespace */
